#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

void delay(unsigned int nCount);

GPIO_InitTypeDef GPIO_InitStruct;
int cycle = 0;

int main(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// One shaft revolution CW using half step mode
	for (cycle = 0; cycle < 512; cycle++)
	{
		GPIO_Write(GPIOB, 0x9000);
		delay(5);
		GPIO_Write(GPIOB, 0x1000);
		delay(5);
		GPIO_Write(GPIOB, 0x3000);
		delay(5);
		GPIO_Write(GPIOB, 0x2000);
		delay(5);
		GPIO_Write(GPIOB, 0x6000);
		delay(5);
		GPIO_Write(GPIOB, 0x4000);
		delay(5);
		GPIO_Write(GPIOB, 0xC000);
		delay(5);
		GPIO_Write(GPIOB, 0x8000);
		delay(5);
	}
	
	delay(1000);
	
	// One shaft revolution CCW using half step mode
	for (cycle = 0; cycle < 512; cycle++)
	{
		GPIO_Write(GPIOB, 0x8000);
		delay(5);
		GPIO_Write(GPIOB, 0xC000);
		delay(5);
		GPIO_Write(GPIOB, 0x4000);
		delay(5);
		GPIO_Write(GPIOB, 0x6000);
		delay(5);
		GPIO_Write(GPIOB, 0x2000);
		delay(5);
		GPIO_Write(GPIOB, 0x3000);
		delay(5);
		GPIO_Write(GPIOB, 0x1000);
		delay(5);
		GPIO_Write(GPIOB, 0x9000);
		delay(5);
	}
	
	while (1)
	{
	}
}

void delay(unsigned int nCount)
{
	unsigned int i, j;
	
	for (i = 0; i < nCount; i++)
		for (j = 0; j < 0x2AFF; j++);
}
