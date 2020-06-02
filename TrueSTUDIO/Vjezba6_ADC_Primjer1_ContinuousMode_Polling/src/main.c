/*
 * Program koji ovisno o visini napona na PA_0 nozici, pali zelenu, zutu,
 * ili crvenu LED. Nozica PA_0 konfigurirana je kao analogni ulaz na koji
 * je spojen klizac potenciometra 10 KOhma kojim se mijenja visina napona
 * na analognom ulazu. Ako je napon u intervalu [0, Vcc/3) pali se zelena
 * LED spojena na PB_14. U slucaju da je napon u intervalu [Vcc/3, 2*Vcc/3]
 * pali se zuta LED, a u intervalu (2Vcc/3, Vcc] pali se crvena LED.
 */

#include <stddef.h>
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"

#define POTENTIOMETER	GPIO_Pin_0
#define LED_RED 	GPIO_Pin_12
#define LED_YELLOW 	GPIO_Pin_13
#define LED_GREEN 	GPIO_Pin_14
#define MAX_ADC_VALUE 0x0FFF

#define LED_ON		Bit_RESET
#define LED_OFF		Bit_SET

void initLEDs(void);
void initADC(void);
void calibrateADC();

int main(void) {

	uint16_t potValue = 0;
	// inicijalizacijski dio
	initLEDs();
	initADC();
	calibrateADC();

	// ADC je konfiguriran da se pokrece putem SW komande
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	// ciklicki dio
	while(1) {
		// cekaj dok ADC konverzija ne zavrsi
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
		// procitaj 12-bitni podatak iz ADC data registra
		potValue = ADC_GetConversionValue(ADC1);

		if (potValue < (MAX_ADC_VALUE / 3)) {
			GPIO_WriteBit(GPIOB, LED_RED, LED_OFF);
			GPIO_WriteBit(GPIOB, LED_YELLOW, LED_OFF);
			GPIO_WriteBit(GPIOB, LED_GREEN, LED_ON);
		} else if((potValue > (2 * MAX_ADC_VALUE) / 3)) {
			GPIO_WriteBit(GPIOB, LED_RED, LED_ON);
			GPIO_WriteBit(GPIOB, LED_YELLOW, LED_OFF);
			GPIO_WriteBit(GPIOB, LED_GREEN, LED_OFF);
		} else {
			GPIO_WriteBit(GPIOB, LED_RED, LED_OFF);
			GPIO_WriteBit(GPIOB, LED_YELLOW, LED_ON);
			GPIO_WriteBit(GPIOB, LED_GREEN, LED_OFF);
		}
	}
}

void initLEDs(void) {
	GPIO_InitTypeDef gpioInitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_StructInit(&gpioInitStruct);
	gpioInitStruct.GPIO_Pin = LED_RED | LED_YELLOW | LED_GREEN;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	// sve LED inicijalno su ugasene
	GPIO_WriteBit(GPIOB, LED_RED, LED_OFF);
	GPIO_WriteBit(GPIOB, LED_YELLOW, LED_OFF);
	GPIO_WriteBit(GPIOB, LED_GREEN, LED_OFF);
	GPIO_Init(GPIOB, &gpioInitStruct);
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
