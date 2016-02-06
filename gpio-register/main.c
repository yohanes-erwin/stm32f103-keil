#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

int main(void)
{
	// Enable peripheral clock for GPIOA
	RCC->APB2ENR |= 0x00000004;
	
	// Configure PA0 as output push-pull, 2MHz speed
	// Configure PA1 as input with pull-up
	// Leave other pin as input floating
	GPIOA->CRL = 0x44444482;
	// Activate internal pull-up for PA1
	GPIOA->ODR |= GPIO_Pin_1;

	while (1)
	{
		// If button on PA1 is pressed (button circuit is active low)
		if (!(GPIOA->IDR & GPIO_Pin_1))
		{
			// Turn on LED on PA0 (LED circuit is active low)
			// Reset PA0
			GPIOA->BRR = 0x01;
		}
		else
		{
			// Turn off LED on PA0
			// Set PA0
			GPIOA->BSRR = 0x01;
		}
	}
}
