/*
 * Program konfigurira i koristi timer TIM2 za generiranje prekida
 * (engl. interrupts) frekvencijom 2 Hz (svakih 0.5 s).
 * Na svaki prekid mijenja se stanje ugradjene LED spojene na PC_13.
 */
#include <stddef.h>
#include "stm32f10x.h"
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>

void initLED(void);
void initTimer(void);
void configTimerInterrupt(void);

int main() {

	// inicijalizacijski dio
	initLED();
	initTimer();
	configTimerInterrupt();

	// ciklicki dio
	while(1) {
		// Sve se logika izvrsava u prekidnoj rutini timera TIM2
	}
}

void initLED(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef gpioInitStruct;
    gpioInitStruct.GPIO_Pin =  GPIO_Pin_13;
    gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    // inicijalno ce LED biti ugasena (spojena je tako da se pali na '0')
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
    GPIO_Init(GPIOC, &gpioInitStruct);
}

/*
 * Vazno: Reference manual str 93:
 * Timer TIM2 okida se frekvencijom 72MHz
 * (Iako je TIM2 spojen na APB1, RCC mnozi APB1 sa 2 prije dolaska na TIM2).
 */

 /* Timer se inicijalizira da generira 2 interrupta u sekundi (2 Hz)
  * (Svakih pola sekunde mijenjamo stanje LED)
  * prescaler: 72 MHz => 10 kHz
  * counter: 10 kHz / 5000 => 2 Hz
  */
void initTimer(void) {

	TIM_TimeBaseInitTypeDef timeBaseInitStruct;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStructInit(&timeBaseInitStruct);
    // frekvencija na izlazu preskalera je 10 kHz
    timeBaseInitStruct.TIM_Prescaler = SystemCoreClock / 10000 - 1;
    // timer broji od 0 do CNT (TIM_Period), dakle dijeli sa 5000
    // 10000 : 5000 = 2Hz
    timeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    timeBaseInitStruct.TIM_Period = 5000 - 1;
    timeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &timeBaseInitStruct);

    // pokreni Timer
    TIM_Cmd(TIM2, ENABLE);

    // omoguci prekida na timer update event (cnt reload)
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

/*
 * Prekidna rutina za TIM2 timer
 */
void TIM2_IRQHandler(void) {
	static uint32_t cnt = 0;

	// provjeri da li je timer-update generirao interrupt
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        // promijeni stanje LED
        if(cnt % 2 == 0) {
        	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
        } else {
        	GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
        }
        cnt++;
    }
}
