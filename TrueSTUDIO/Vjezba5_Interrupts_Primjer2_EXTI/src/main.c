/*
 * Program koji na svaki pritisak tipkala mijenja stanje LED
 * Tipkalo je spojeno na PA_0 nozicu, koju treba konfigurirati kao
 * nozicu preko koje se mogu generirati prekidi (engl. interrupts).
 * LED je spojen na nozicu PB_1.
 */

#include <stddef.h>
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"

// prototipovi funkcija za inicijalizaciju
void initLED(void);
void initEXTI1(void);
void initNVIC(void);

volatile int detektiranEXTI0 = 0;

int main(void) {

	BitAction ledStatus = Bit_SET;

	// inicijalizacijski dio
	initLED();
	initEXTI1();
	initNVIC();
	GPIO_WriteBit(GPIOB, GPIO_Pin_1, ledStatus);

	// ciklicki dio
	while(1) {
		if(detektiranEXTI0 == 1) {
			// omoguci detekciju novog prekida
			detektiranEXTI0 = 0;
			// izmijeni stanje na LED
			ledStatus = (ledStatus == Bit_SET) ? Bit_RESET : Bit_SET;
			GPIO_WriteBit(GPIOB, GPIO_Pin_1, ledStatus);
		}
	}
}

// funkcija za inicijalizaciju izlaznog porta na koji je spojena LED
void initLED(void) {
	GPIO_InitTypeDef gpioInitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_StructInit(&gpioInitStruct);
	gpioInitStruct.GPIO_Pin = GPIO_Pin_1;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	// pocetno stanje na izlazu je 1 (LED je spojena tako da tada ne svijetli)
	GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_SET);
	GPIO_Init(GPIOB, &gpioInitStruct);
}

// funkcija za inicijalizaciju ulaznog porta na koji je spojeno tipkalo
// kao porta (nozice) za generiranje prekida
void initEXTI1(void) {
	GPIO_InitTypeDef gpioInitStruct;
	EXTI_InitTypeDef extiInitStruct;

	// 1-Inicijalizacija RCC: clock treba dovesti na GPIOA i AFIO
	// Nozica PA_0 koristi se za prekide (alternatvna funkcija te nozice)
	// pa je potrebno clock dovesti i na AFIO modul
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

	// 2-Inicijalizacija PA0 za funkciju generiranja vanjskog prekida EXTI0
	GPIO_StructInit(&gpioInitStruct);
	gpioInitStruct.GPIO_Pin = GPIO_Pin_0;
	// tipkalo je spojeno na +3.3V preko internog pull-up otpornika
	gpioInitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &gpioInitStruct);

	// 3-Povezivanje vanjske PA_0 nozice na internu EXTI0 liniju (konfiguracija MUX-a)
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

	// 4-Inicijalizacija EXTI0 linije : nozica generira interrupt na padajuci brid signala
	extiInitStruct.EXTI_Line = EXTI_Line0;
	extiInitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	extiInitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	extiInitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&extiInitStruct);
}

void initNVIC(void) {
	NVIC_InitTypeDef nvicInitStruct;

	// sva 4 bita koriste se za sub-priority (svi prekidi imaju isti prioritet)
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	// dodijeli EXTI0 prekidu sub-prioritet 3 i omoguci prekide s te linije
	nvicInitStruct.NVIC_IRQChannel = EXTI0_IRQn;
	nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	nvicInitStruct.NVIC_IRQChannelSubPriority = 3;
	nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&nvicInitStruct);
}

// prekidni potprogram za obradu EXTI0 prekida
// (engl. EXTI0 ISR (Interrupt Service Routine)
void EXTI0_IRQHandler(void) {
	detektiranEXTI0 = 1;
	// obrisi zastavicu (flag) prekida - time potvrdjujemo da je prekid obradjen
	EXTI_ClearITPendingBit(EXTI_Line0);
}
