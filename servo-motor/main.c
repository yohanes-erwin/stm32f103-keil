#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "delay.h"
#include "lcd16x2.h"

#define DEGREE_0			0
#define DEGREE_45			45
#define DEGREE_90			90
#define DEGREE_135		135
#define DEGREE_180		180

void PWM_Init(void);
void displayServoPosition(int degree);

int main(void)
{
	DelayInit();
	lcd16x2_init(LCD16X2_DISPLAY_ON_CURSOR_OFF_BLINK_OFF);
	
	// Initialize PWM
	PWM_Init();
	
	while (1)
	{
		// Set servo position to 0 degree
		displayServoPosition(DEGREE_0);
		TIM2->CCR1 = 520;
		DelayMs(2000);
		// Set servo position to 45 degree
		displayServoPosition(DEGREE_45);
		TIM2->CCR1 = 840;
		DelayMs(2000);
		// Set servo position to 90 degree
		displayServoPosition(DEGREE_90);
		TIM2->CCR1 = 1140;
		DelayMs(2000);
		// Set servo position to 135 degree
		displayServoPosition(DEGREE_135);
		TIM2->CCR1 = 1500;
		DelayMs(2000);
		// Set servo position to 180 degree
		displayServoPosition(DEGREE_180);
		TIM2->CCR1 = 1800;
		DelayMs(2000);
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
	// Create 50Hz PWM
	// Prescale timer clock from 72MHz to 720kHz by prescaler = 100
	TIM_TimeBaseInitStruct.TIM_Prescaler = 100;
	// TIM_Period = (timer_clock / PWM_frequency) - 1
	// TIM_Period = (720kHz / 50Hz) - 1 = 14399
	TIM_TimeBaseInitStruct.TIM_Period = 14399;
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

void displayServoPosition(int degree)
{
	// Display servo position to the LCD
	lcd16x2_clrscr();
	if (degree == DEGREE_0)
	{
		lcd16x2_puts("0");
	}
	else if (degree == DEGREE_45)
	{
		lcd16x2_puts("45");
	}
	else if (degree == DEGREE_90)
	{
		lcd16x2_puts("90");
	}
	else if (degree == DEGREE_135)
	{
		lcd16x2_puts("135");
	}
	else if (degree == DEGREE_180)
	{
		lcd16x2_puts("180");
	}
	lcd16x2_putc(0xDF);
}
