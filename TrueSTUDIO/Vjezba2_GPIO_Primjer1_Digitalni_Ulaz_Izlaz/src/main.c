#include <stddef.h>
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

// prototipovi funkcija za inicijalizaciju
void initTipkalo(void);
void initLED(void);

int main() {
	// varijabla za pohranu stanja tipkala
	uint8_t stanje;

	// inicijalizacija ulaznog i izlaznog porta
	initTipkalo();
	initLED();

	// citanje stanja tipkala i azuriranje stanja LED
	while(1) {
		stanje = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
		GPIO_WriteBit(GPIOB, GPIO_Pin_1, stanje);
	}
}

// funkcija za inicijalizaciju ulaznog porta na koji je spojeno tipkalo
void initTipkalo(void) {
	GPIO_InitTypeDef gpioInitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_StructInit(&gpioInitStruct);
	gpioInitStruct.GPIO_Pin = GPIO_Pin_0;
	// tipkalo je spojeno tako da se na ulazu cita '1' kad nije stisnuto
	gpioInitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &gpioInitStruct);
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
