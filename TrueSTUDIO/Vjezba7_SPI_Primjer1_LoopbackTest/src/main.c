/*
 * Program prikazuje osnovni koristenje SPI modula mikrokontrolera.
 * Koristi se SPI1 modul na nacin da se medjusobno spoje SPI1_MOSI (PA_7)
 * i SPI1_MISO (PA_6) nozice (tzv. loopback test). Svaki podatak koji se
 * posalje preko SPI1_MOSI treba se primiti na SPI1_MISO nozici.
 *
 * Program je najpoucnije izvoditi u Step-by-Step debug modu i pratiti
 * vrijednosti varijabli txBuffer, rxBuffer te stanje SS nozice (preko LED)
 */

#include <stddef.h>
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"

// Nozica PB_1 ce se koristiti kao Slave Select za SPI1
#define SS GPIO_Pin_1
#define BUFF_SIZE 4

void initSPI(void);
int spi1ReadWriteBytes(uint8_t *txBuff, uint8_t *rxBuff, uint8_t count);

uint8_t txBuffer[BUFF_SIZE] = {3, 5, 7, 11};
uint8_t rxBuffer[BUFF_SIZE] = {0};

int main(void) {
	// inicijalizacijski dio
	initSPI();

	// ciklicki dio
	while(1) {
		GPIO_WriteBit(GPIOB, SS, Bit_RESET);
		spi1ReadWriteBytes(txBuffer, rxBuffer, BUFF_SIZE);
		GPIO_WriteBit(GPIOB, SS, Bit_SET);
	}
}

// funkcija za inicijalizaciju SPI1
void initSPI(void) {
	GPIO_InitTypeDef gpioInitStruct;
	SPI_InitTypeDef spiInitStruct;

	// 1-Inicijalizacija RCC: clock treba dovesti na GPIOA, GPIOB, AF i SPI1 blokove
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, ENABLE);

	// 2a-Inicijalizacija GPIO za SCK:  SPI1_SCK  (PA_5, AF push-pull)
	//    Inicijalizacija GPIO za MOSI: SPI1_MOSI (PA_7, AF push-pull)
	GPIO_StructInit(&gpioInitStruct);
	gpioInitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpioInitStruct);

	// 2b-Inicijalizacija GPIO za MISO: SPII1_MISO (PA_6, IN floating/pull up)
	gpioInitStruct.GPIO_Pin = GPIO_Pin_6;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpioInitStruct);

	// 2c-Inicijalizacija GPIO za SS (Slave Select) (PB_1, OUT push-pull, active low)
	gpioInitStruct.GPIO_Pin = SS;
	gpioInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_WriteBit(GPIOB, SS, Bit_SET); // inicijalno na '1' (neaktivna razina za SS)
	GPIO_Init(GPIOB, &gpioInitStruct);

	// 3-Inicijalizacija SPI modula
	SPI_StructInit(&spiInitStruct);
	// SPI clock = 72 MHz / 64 = 1.125 MHz
	spiInitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	spiInitStruct.SPI_Mode = SPI_Mode_Master;
	spiInitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	spiInitStruct.SPI_DataSize = SPI_DataSize_8b;
	// SPI mode 00
	spiInitStruct.SPI_CPOL = SPI_CPOL_Low;
	spiInitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	// prvi bit podatka koji se salje je MSB (bit 7)
	spiInitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	// postavljanje SS nozice radit cemo programskim putem
	// mozemo izabrati bilo koji GPIO pin za SS i konfigurirati ga za OUT push-pull
	spiInitStruct.SPI_NSS = SPI_NSS_Soft;

	SPI_Init(SPI1, &spiInitStruct);

	// pokreni SPI1
	SPI_Cmd(SPI1, ENABLE);
}

int spi1ReadWriteBytes(uint8_t *txBuff, uint8_t *rxBuff, uint8_t count) {
	int i = 0;

	for(i = 0; i < count; i++) {

		// TX: ako se ne zeli slati podatke, postavi txBuff na NULL pri pozivu spi1ReadWriteBytes() funkcije
		if(txBuff != NULL) {
			SPI_I2S_SendData(SPI1, *txBuff++);
		} else {
			// posto se mora nesto poslati da bi se nesto moglo primiti, saljemo 0xFF
			SPI_I2S_SendData(SPI1, 0xFF);
		}

		// RX: Cekaj dok se ne primi podatak
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
		// ako primljeni podatak ne zelimo pohraniti, postavi rxBuff na NULL pri pozivu spi1ReadWriteBytes() funkcije
		if(rxBuff != NULL) {
			*rxBuff++ = SPI_I2S_ReceiveData(SPI1);
		} else {
			SPI_I2S_ReceiveData(SPI1);
		}
	}
	return i;
}
