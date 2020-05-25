/*
 * Program koji svaki znak koji dolazi na UART1 Rx nozicu (PA_10)
 * salje na UART1_Tx nozicu.
 * Napomena 1: ne moze se slati znak cija je vrijednost 0xFF jer se ta vrijednost
 * koristi kao informacija da nema primljenog znaka na Rx nozici
 * Napomena 2: USB to RS232/TTL adapter spojiti sa BluePill na slijedeci nacin:
 *  Tx zica (zelena) spojena na PA_10 nozicu (USART1 Rx)
 *  Rx zica (bijela) spojena na PA_9 nozicu (USART1 Tx)
 *  GND zica (crna) spojena na GND nozicu BleuPill
 *  +5V zica (crvena) se ne spaja na BluePill
 */
#include <stddef.h>
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"

// prototipovi funkcija
void initUSART(void);
uint8_t getChar(void);
void putChar(uint8_t c);

int main(void)
{
	uint8_t value;
	// inicijalizacijski dio
	initUSART();

	// ciklicki dio
	while(1) {
		value = getChar();
		if (value != 0xFF) {
			putChar(value);
		}
	}
}

// funkcija za inicijalizaciju USART1
void initUSART(void) {
	GPIO_InitTypeDef gpioInitStruct;
	USART_InitTypeDef usartInitStruct;

	// 1-Inicijalizacija RCC: clock treba dovesti na GPIOA, AF i USART1 blokove
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);

	// 2a-Inicijalizacija GPIO za Tx: USART1_Tx(PA_9, AF OUT push-pull)
	GPIO_StructInit(&gpioInitStruct);
	gpioInitStruct.GPIO_Pin = GPIO_Pin_9;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &gpioInitStruct);

	// 2b-Inicijalizacija GPIO za Rx: USART1_Rx(PA_10, AF IN floating/pull-up)
	gpioInitStruct.GPIO_Pin = GPIO_Pin_10;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpioInitStruct);

	// 3-Inicijalizacija USART1
	USART_StructInit(&usartInitStruct);
	usartInitStruct.USART_BaudRate = 115200;
	usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // omoguci i primanje i slanje
	usartInitStruct.USART_WordLength = USART_WordLength_8b;
	usartInitStruct.USART_Parity = USART_Parity_No;
	usartInitStruct.USART_StopBits = USART_StopBits_1;
	usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1, &usartInitStruct);

	// pokretanje USART1
	USART_Cmd(USART1, ENABLE); // pokretanje USART1
}

// funkcija za citanje jednog znaka sa serijskog porta
uint8_t getChar(void) {
	uint8_t retVal = 0xFF;
	// ako je znak stigao procitaj ga, ako nije vrati 0xFF
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET) {
		retVal = USART_ReceiveData(USART1);
	}
	return retVal;
}

// funkcija za slanje jednog znaka na serijski port
void putChar(uint8_t c) {
	// cekaj dok/ako je slanje prethodnog znaka u tijeku
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {}
	USART_SendData(USART1, c);
}
