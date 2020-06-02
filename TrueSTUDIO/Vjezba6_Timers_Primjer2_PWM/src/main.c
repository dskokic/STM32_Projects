/*
 * Program koji ovisno o visini napona na PA_0 nozici, regulira intenzitet
 * svjetla na zelenoj LED spojenoj na PA_1 nozicu (PWM2/2) i na zutoj LED
 * spojenoj na PA_2 nozicu (PWM2/3). Nozica PA_0 konfigurirana
 * je kao analogni ulaz na koji je spojen klizac potenciometra 10 KOhma
 * kojim se mijenja visina napona na analognom ulazu.
 * Ovisno o procitanom naponu mijenja se PWM "duty cycle" Timera na kanalima 2 i 3
 * na koje su spojene LED te se tako mijenja intenzitet svjetla.
 * Program je zamisljen da kad se intenzitet jedne LED povecava druge se smanjuje
 * i obruto.
 *
 * Program konfigurira i koristi timer TIM2 za generiranje prekida frekvencijom
 * 100 Hz (svakih 10 ms). Na svaki prekid omoguceno se mijenjanje intenziteta LED.
 */

#include <stddef.h>
#include "stm32f10x.h"
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_adc.h>

#define POTENTIOMETER	GPIO_Pin_0

void initLED(void);
void initTimer(void);
void configTimerInterrupt(void);
void initADC(void);
void calibrateADC();

volatile int isUpdateTime = 0;

int main() {
	uint16_t intensity = 0;

	// inicijalizacijski dio
	initLED();
	initTimer();
	configTimerInterrupt();
	initADC();
	calibrateADC();

	// ADC je konfiguriran da se pokrece putem SW komande
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	TIM_SetCompare2(TIM2, 30);
	TIM_SetCompare3(TIM2, 30);
	// ciklicki dio
	while(1) {

		// cekaj dok ADC konverzija ne zavrsi
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
		// procitaj 12-bitni podatak iz ADC data registra
		intensity = ADC_GetConversionValue(ADC1);
		// normaliziraj na 10 bita do maksimalne vrijednosti 1000
		intensity = (intensity >> 2) ; //4095 >> 2 = 1023
		// za slucaj da je intensity u intervalu (1000, 1023]
		if(intensity > 1000) {
			intensity = 1000;
		}

		// azuriraj intenzitet
		if (isUpdateTime == 1) {
			TIM_SetCompare2(TIM2, intensity);
			TIM_SetCompare3(TIM2, 1000 - intensity);
			isUpdateTime = 0;
			//for (int i = 0; i < 500000; i++);
		}
	}
}

void initLED(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef gpioInitStruct;
    gpioInitStruct.GPIO_Pin =  GPIO_Pin_1 | GPIO_Pin_2;
    gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    // inicijalno ce LED biti ugasene (spojene su tako da se pale na '0')
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);
    GPIO_WriteBit(GPIOA, GPIO_Pin_2, Bit_SET);
    GPIO_Init(GPIOA, &gpioInitStruct);
}

/*
 * Vazno: Reference manual str 93:
 * Timer TIM2 okida se frekvencijom 72MHz
 * (Iako je TIM2 spojen na APB1, RCC mnozi APB1 sa 2 prije dolaska na TIM2).
 */

 /* Timer se inicijalizira da generira  100 prekida u sekundi (100 Hz)
  * prescaler: 72 MHz => 100 kHz
  * counter: 100 kHz / 1000 => 100 Hz
  */
void initTimer(void) {
	TIM_TimeBaseInitTypeDef timeBaseInitStruct;
	TIM_OCInitTypeDef       TIM_OCInitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStructInit(&timeBaseInitStruct);
    // frekvencija na izlazu preskalera je 100 kHz
    timeBaseInitStruct.TIM_Prescaler = SystemCoreClock / 100000 - 1;
    // 100000 : 1000 = 100Hz
    timeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    timeBaseInitStruct.TIM_Period = 1000 - 1;
    timeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &timeBaseInitStruct);

	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OCMode      = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC2Init(TIM2, &TIM_OCInitStruct); // ch2
	TIM_OC3Init(TIM2, &TIM_OCInitStruct); // ch3

    // pokreni Timer
    TIM_Cmd(TIM2, ENABLE);

    // omoguci prekid na timer update event (cnt reload)
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

}

void configTimerInterrupt(void) {
    NVIC_InitTypeDef nvicInitStruct;

	// sva 4 bita koriste se za sub-priority (svi prekidi imaju isti prioritet)
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	nvicInitStruct.NVIC_IRQChannel = TIM2_IRQn;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInitStruct.NVIC_IRQChannelSubPriority = 1;
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInitStruct);
}

void initADC(void) {
	GPIO_InitTypeDef gpioInitStruct;
	ADC_InitTypeDef	 adcInitStruct;

	// ADC moeze raditi na max 14 MHz (72 MHz/6 = 12 MHz, sto je ok)
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	// 1-Inicijalizacija RCC: clock treba dovesti na GPIOA i ADC1
	// Nozica PA_0 koristi kao analogni ulaz
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1 , ENABLE);

	// 2-Inicijalizacija PA0 za funkciju analognog ulaza (AIN)
	GPIO_StructInit(&gpioInitStruct);
	gpioInitStruct.GPIO_Pin = POTENTIOMETER;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &gpioInitStruct);

	// 3-Inicijalizacija ADC-a
	// konfiguracija: single channel, continous conversion,
	// no external triger to start conversion (started by calling SW function)
	ADC_StructInit(&adcInitStruct);
	adcInitStruct.ADC_NbrOfChannel = 1;
	adcInitStruct.ADC_Mode = ADC_Mode_Independent;
	adcInitStruct.ADC_ContinuousConvMode = ENABLE;
	adcInitStruct.ADC_ScanConvMode = DISABLE;
	adcInitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	adcInitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_Init(ADC1, &adcInitStruct);

	// 4-Konfiguriranje analognog ulaza PA0: PA0 je ADC-u vidljiv kao ADC_Channel_0
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);

	// 5-Pokreni ADC
	ADC_Cmd(ADC1, ENABLE);
}

// ADC zahtjeva kalibraciju radi tocnosti konverzije (samo jednom prilikom pokretanja)
void calibrateADC(void) {
	while(ADC_GetResetCalibrationStatus(ADC1)); // Inicijaliziraj registar za kalibraciju
	ADC_StartCalibration(ADC1); // Pokreni kalibraciju
	while(ADC_GetCalibrationStatus(ADC1)); // Cekaj dok kalibracija ne zavrsi
}


/*
 * Prekidna rutina za TIM2 timer
 * Okida se ciklicki, na svakom reload-u timera TIM2
 * (f = 100 Hz)
 */
void TIM2_IRQHandler(void) {

	// provjeri da li je timer-update generirao interrupt
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
    	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    	isUpdateTime = 1;
    }
}
