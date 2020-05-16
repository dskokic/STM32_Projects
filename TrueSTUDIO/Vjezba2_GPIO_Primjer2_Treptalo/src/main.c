#include <stddef.h>
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

// prototipovi funkcija
void initLED(void);
void pauza(int brojac);

int main() {
	// inicijalizacija LED porta (PB_1) kao izlaznog
	initLED();

	// treptalo
	while(1) {
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_RESET);
		pauza(5000000);
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, Bit_SET);
		pauza(5000000);

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

// funkcija za pauziranje
void pauza(int brojac) {
	int i = brojac;

	// jednostavno trosenje odredjenog broja procesorskih ciklusa
	while (i > 0) {
		i--;
	}
	return;
}
