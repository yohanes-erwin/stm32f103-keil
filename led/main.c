#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

void delay(unsigned int nCount);

GPIO_InitTypeDef GPIO_InitStruct;

int main (void)
{
	// Enable clock for GPIOA
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	// Configure PA0 as push-pull output
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	while (1)
	{
		/* Toggle LED on PA0 */
		// Reset bit will turn on LED (because the logic is interved)
		GPIO_ResetBits(GPIOA, GPIO_Pin_0);
		delay(1000);
		// Set bit will turn off LED (because the logic is interved)
		GPIO_SetBits(GPIOA, GPIO_Pin_0);
		delay(1000);
	}
}

// Delay function
void delay(unsigned int nCount)
{
	unsigned int i, j;
	
	for (i = 0; i < nCount; i++)
		for (j = 0; j < 0x2AFF; j++);
}