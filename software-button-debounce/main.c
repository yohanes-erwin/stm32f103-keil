#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "delay.h"

static __inline void debounce(void);

GPIO_InitTypeDef GPIO_InitStruct;
volatile uint8_t button_pressed = 0;

int main(void)
{
	DelayInit();
	
	// Enable clock for GPIOB
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	// Initialize PB12 as input with pull-up resistor
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	// Initialize PB13 as push-pull output
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	while (1)
	{
		// Update button_state
		debounce();
		// Check if the button is pressed
		if (button_pressed)
		{
			// Clear flag
			button_pressed = 0;
			// Toggle LED on PB13
			GPIOB->ODR ^= GPIO_Pin_13;
		}
		// Delay for a while so we don't check to button too often
		DelayMs(10);
	}
}

static __inline void debounce(void)
{
	// Counter for number of equal states
	static uint8_t count = 0;
	// Keep track of current (debounced) state
	static uint8_t button_state = 0;

	// Check if button is high or low for the moment
	uint8_t current_state = (GPIOB->IDR & GPIO_Pin_12) == 0;
	
	if (current_state != button_state)
	{
		// Button state is about to be changed, increase counter
		count++;
		if (count >= 4)
		{
			// The button have not bounced for four checks, change state
			button_state = current_state;
			// If the button was pressed (not released), tell main so
			if (current_state != 0)
			{
				button_pressed = 1;
			}
			count = 0;
		}
	}
	else
	{
		// Reset counter
		count = 0;
	}
}
