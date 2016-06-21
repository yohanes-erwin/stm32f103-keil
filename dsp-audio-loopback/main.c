/**
  ******************************************************************************
	* @file		main.c
  * @author	Yohanes Erwin Setiawan
  * @date		23 May 2016 
	******************************************************************************
	* @brief	Audio loop (Read ADC value then write back to PWM)
	* 				1. ADC: 
	*							- ADC1 channel 1 (PA1) (10-bit)
	*					2. TIMER:
	*							- TIM2
	*							- Interrupt rate = 35.15kHz (sampling frequency)
	*							- PWM frequency = 35.15kHz 
	*							- PWM pin PA0 (10-bit)
	*					3. USART:
	*							- USART3
	*							- Tx pin PB10
	******************************************************************************
	*/

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "delay.h"
#include <stdio.h>

// If DEBUG = 1, then ADC value is sent to UART (for debugging)
#define DEBUG		1

uint16_t adcValue;
char sAdcValue[6];

void ADC_Setup(void);
void PWM_Setup(void);
void UART_Setup(void);
uint16_t ADC_Read(void);
void PWM_Write(uint16_t val);
void UART_PutChar(char c);
void UART_PutString(char *s);

void TIM2_IRQHandler()
{
	// Checks whether the TIM2 interrupt has occurred or not
	if (TIM_GetITStatus(TIM2, TIM_IT_Update))
	{
		// Read ADC value (10-bit PWM)
		adcValue = ADC_Read() >> 2;
		// Write to PWM
		PWM_Write(adcValue);
		
		// Clears the TIM2 interrupt pending bit
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

int main(void)
{
	// Initialize delay function
	DelayInit();
	
	// Initialize ADC, USART, and PWM
	ADC_Setup();
	if (DEBUG)
	{
		UART_Setup();
	}
	PWM_Setup();
	
	while (1)
	{
		if (DEBUG)
		{
			// Send ADC value to UART for debugging
			sprintf(sAdcValue, "%i\n", adcValue);
			UART_PutString(sAdcValue);
			DelayMs(500);
		}
	}
}

void ADC_Setup()
{
	// Initialization struct
	ADC_InitTypeDef ADC_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Step 1: Initialize ADC1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConv = DISABLE;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStruct.ADC_NbrOfChannel = 1;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC1, &ADC_InitStruct);
	ADC_Cmd(ADC1, ENABLE);
	// Select input channel for ADC1
	// ADC1 channel 1 (PA1)
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_7Cycles5);
	
	// Step 2: Initialize GPIOA (PA1)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void PWM_Setup()
{
	// Initialization struct
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Step 1: Initialize TIM2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	// Timer freq = timer_clock / ((TIM_Prescaler+1) * (TIM_Period+1))
	// Timer freq = 72MHz / ((1+1) * (1023+1) = 35.15kHz
	TIM_TimeBaseInitStruct.TIM_Prescaler = 1;
	TIM_TimeBaseInitStruct.TIM_Period = 1023;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	// Enable TIM2 interrupt
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

	// Step 2: Initialize NVIC
	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	// Step 3: Initialize PWM
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_Pulse = 0;
	TIM_OC1Init(TIM2, &TIM_OCInitStruct);
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
	
	// Step 4: Initialize GPIOA (PA0)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// Initialize PA0 as push-pull alternate function (PWM output)
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void UART_Setup()
{
	// Initialization struct
	USART_InitTypeDef USART_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Step 1: USART3 initialization
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	USART_InitStruct.USART_BaudRate = 9600;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART3, &USART_InitStruct);
	USART_Cmd(USART3, ENABLE);
	
	// Step 2: GPIO initialization for Tx (PB10) pin
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	// Tx pin initialization as push-pull alternate function
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
}

uint16_t ADC_Read()
{
	// Start ADC conversion
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	// Wait until ADC conversion finished
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

	return ADC_GetConversionValue(ADC1);
}

void PWM_Write(uint16_t val)
{
	// Write PWM value
	TIM2->CCR1 = val;
}

void UART_PutChar(char c)
{
	// Wait until transmit data register is empty
	while (!USART_GetFlagStatus(USART3, USART_FLAG_TXE));
	// Send a char using USART3
	USART_SendData(USART3, c);
}

void UART_PutString(char *s)
{
	// Send a string
	while (*s)
	{
		UART_PutChar(*s++);
	}
}
