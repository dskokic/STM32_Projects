/*
 * Vazno: obrisi ili zakomentiraj funkciju SysTick_Handler u
 * datoteci stm32f1xx_it.c (u istom je direktoriju kao i main.c)
 * jer je SysTick_Handler funkcija implementirana ovdje u main.c
 * datoteci (inace ce linker prijaviti gresku jer nalazi 2 funkcije
 * istog naziva
 *
 * Program koristi SysTick timer za generiranje preciznih vremenskih intervala.
 * Prekidi (interrupti) SysTick timera generiraju se ovdje svake milisekunde.
 * U konkretnom primjeru je to iskoristeno za precizno odredjivanje trajanja pauze
 * prilikom promjene stanja LED koja je spojena na PB_1 nozicu.
 */

#include <stddef.h>
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

// prototipovi funkcija za inicijalizaciju
void initLED(void);
// funkcija pauza(), argument je trajanje pauze u milisekundama
void pauza(int vrijeme);

volatile int ms = 0;

int main(void) {

	// inicijalizacijski dio
	initLED();

	// argument za SysTick_Config() funkciju jest broj
	// s kojim ce se dijeliti SystemCoreClock (72 MHz)
	// Tako dobiveni broj jest frekvencija SysTicka u Hz.
	// Konkretno: SystemCoreClock / (SystemCoreClock/1000) = 1000 Hz
	SysTick_Config(SystemCoreClock/1000);

	// ciklicki dio
	while(1) {
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_RESET);
		pauza(100);
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_SET);
		pauza(900);
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

void pauza(int vrijeme) {
	ms = vrijeme;
	while(ms != 0);
}

// prekidni potprogram SysTick timera
// (engl. SysTick ISR (Interrupt Service Routine)
void SysTick_Handler(void) {
	if(ms != 0) {
		ms--;
	}
}

