#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "delay.h"

GPIO_InitTypeDef GPIOInitStruct;

int main(void)
{
	// Initialize delay functions
	DelayInit();
	
	// Initialize GPIOA as output for LED
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIOInitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIOInitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIOInitStruct);
	
	while (1)
	{
		/* LED blink every 1s */
		GPIO_ResetBits(GPIOA, GPIO_Pin_0);
		DelayMs(1000);
		GPIO_SetBits(GPIOA, GPIO_Pin_0);
		DelayMs(1000);
	}
}

