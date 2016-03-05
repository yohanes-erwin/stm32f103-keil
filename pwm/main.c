#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "delay.h"

void PWM_Init(void);

int brightness;

int main(void)
{
	DelayInit();
	
	// Initialize PWM
	PWM_Init();
	
	while (1)
	{
		// Set LED brightness from 0 to max
		for (brightness = 0; brightness <= 7199; brightness += 10)
		{
			TIM2->CCR1 = brightness;
			DelayMs(1);
		}
		// Set LED brightness from max to 0
		for (brightness = 7199; brightness >= 0; brightness -= 10)
		{
			TIM2->CCR1 = brightness;
			DelayMs(1);
		}
	}
}

void PWM_Init()
{
	// Initialization struct
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Step 1: Initialize TIM2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	// Create 1kHz PWM
	// TIM2 is connected to APB1 bus that have default clock 72MHz
	// So, the frequency of TIM2 is 72MHz
	// We use prescaler 10 here
	// So, the frequency of TIM2 now is 72MHz
	TIM_TimeBaseInitStruct.TIM_Prescaler = 10;
	// TIM_Period determine the PWM frequency by this equation:
	// PWM_frequency = timer_clock / (TIM_Period + 1)
	// If we want 1kHz PWM we can calculate:
	// TIM_Period = (timer_clock / PWM_frequency) - 1
	// TIM_Period = (7.2MHz / 1kHz) - 1 = 7199
	TIM_TimeBaseInitStruct.TIM_Period = 7199;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	// Start TIM2
	TIM_Cmd(TIM2, ENABLE);
	
	// Step 2: Initialize PWM
	// Common PWM settings
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	// Duty cycle calculation equation:
	// TIM_Pulse = (((TIM_Period + 1) * duty_cycle) / 100) - 1
	// Ex. 25% duty cycle:
	// 		TIM_Pulse = (((7199 + 1) * 25) / 100) - 1 = 1799
	//		TIM_Pulse = (((7199 + 1) * 75) / 100) - 1 = 5399
	// We initialize PWM value with duty cycle of 0%
	TIM_OCInitStruct.TIM_Pulse = 0;
	TIM_OC1Init(TIM2, &TIM_OCInitStruct);
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
	
	// Step 3: Initialize GPIOA (PA0)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// Initialize PA0 as push-pull alternate function (PWM output) for LED
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}
