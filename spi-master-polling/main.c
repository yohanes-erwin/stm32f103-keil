#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "delay.h"
#include "lcd16x2.h"

#define SPIx_RCC				RCC_APB2Periph_SPI1
#define SPIx						SPI1
#define SPI_GPIO_RCC		RCC_APB2Periph_GPIOA
#define SPI_GPIO				GPIOA
#define SPI_PIN_MOSI		GPIO_Pin_7
#define SPI_PIN_MISO		GPIO_Pin_6
#define SPI_PIN_SCK			GPIO_Pin_5
#define SPI_PIN_SS			GPIO_Pin_4

void SPIx_Init(void);
uint8_t SPIx_Transfer(uint8_t data);
void SPIx_EnableSlave(void);
void SPIx_DisableSlave(void);
	
uint8_t receivedByte;

int main(void)
{
	DelayInit();
	lcd16x2_init(LCD16X2_DISPLAY_ON_CURSOR_OFF_BLINK_OFF);
	
	SPIx_Init();
	
	while (1)
	{
		// Enable slave
		SPIx_EnableSlave();
		// Write command to slave to turn on LED blinking
		SPIx_Transfer((uint8_t) '1');
		DelayUs(10);
		// Write command to slave for asking LED blinking status
		SPIx_Transfer((uint8_t) '?');
		DelayUs(10);
		// Read LED blinking status (off/on) from slave by transmitting dummy byte
		receivedByte = SPIx_Transfer(0);
		// Disable slave
		SPIx_DisableSlave();
		// Display LED blinking status
		lcd16x2_clrscr();
		if (receivedByte == 0)
		{
			lcd16x2_puts("LED Blinking Off");
		}
		else if (receivedByte == 1)
		{
			lcd16x2_puts("LED Blinking On");
		}
		DelayMs(2500);
		
		// Enable slave
		SPIx_EnableSlave();
		// Write command to slave to turn off LED blinking
		SPIx_Transfer((uint8_t) '0');
		DelayUs(10);
		// Write command to slave for asking LED blinking status
		SPIx_Transfer((uint8_t) '?');
		DelayUs(10);
		// Read LED blinking status (off/on) from slave by transmitting dummy byte
		receivedByte = SPIx_Transfer(0);
		// Disable slave
		SPIx_DisableSlave();
		// Display LED blinking status
		lcd16x2_clrscr();
		if (receivedByte == 0)
		{
			lcd16x2_puts("LED Blinking Off");
		}
		else if (receivedByte == 1)
		{
			lcd16x2_puts("LED Blinking On");
		}
		DelayMs(2500);
	}
}

void SPIx_Init()
{
	// Initialization struct
	SPI_InitTypeDef SPI_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Step 1: Initialize SPI
	RCC_APB2PeriphClockCmd(SPIx_RCC, ENABLE);
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;
	SPI_Init(SPIx, &SPI_InitStruct); 
	SPI_Cmd(SPIx, ENABLE);
	
	// Step 2: Initialize GPIO
	RCC_APB2PeriphClockCmd(SPI_GPIO_RCC, ENABLE);
	// GPIO pins for MOSI, MISO, and SCK
	GPIO_InitStruct.GPIO_Pin = SPI_PIN_MOSI | SPI_PIN_MISO | SPI_PIN_SCK;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_GPIO, &GPIO_InitStruct);
	// GPIO pin for SS
	GPIO_InitStruct.GPIO_Pin = SPI_PIN_SS;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_GPIO, &GPIO_InitStruct);
	
	// Disable SPI slave device
	SPIx_DisableSlave();
}

uint8_t SPIx_Transfer(uint8_t data)
{
	// Write data to be transmitted to the SPI data register
	SPIx->DR = data;
	// Wait until transmit complete
	while (!(SPIx->SR & (SPI_I2S_FLAG_TXE)));
	// Wait until receive complete
	while (!(SPIx->SR & (SPI_I2S_FLAG_RXNE)));
	// Wait until SPI is not busy anymore
	while (SPIx->SR & (SPI_I2S_FLAG_BSY));
	// Return received data from SPI data register
	return SPIx->DR;
}

void SPIx_EnableSlave()
{
	// Set slave SS pin low
	SPI_GPIO->BRR = SPI_PIN_SS;
}

void SPIx_DisableSlave()
{
	// Set slave SS pin high
	SPI_GPIO->BSRR = SPI_PIN_SS;
}
