/**
  ******************************************************************************
	* @file		main.c
  * @author	Yohanes Erwin Setiawan
  * @date		15 Jun 2016 
	******************************************************************************
	* @brief	Simple spectrum analyzer using 32 point real DFT
	*					Audio loopback (Read ADC value then write back to PWM)
	* 				1. ADC: 
	*							- ADC1 channel 1 (PA1) (10-bit)
	*					2. TIMER:
	*							- TIM3
	*							- Interrupt rate = 35.15kHz (sampling frequency)
	*					3. PWM:
	*							- TIM2
	*							- PWM frequency = 35.15kHz 
	*							- PWM pin PA0 (10-bit)
	*					4. LCD:
	*							- LCD 16x2
	******************************************************************************
	*/

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_tim.h"
#include "delay.h"
#include "lcd16x2.h"
#include "lookup.h"
#include <math.h>

// The real DFT transforms an N point time domain signal 
// into two N/2+1 point frequency domain signals
// 32 point time domain signal 
#define N_TIME	32
// 17 point frequency domain signal
#define N_FREQ	N_TIME/2+1

volatile uint16_t adc_value = 0;
volatile uint8_t n_count = 0;
volatile uint8_t n_done = 0;
uint16_t x[N_TIME];
int REX[N_FREQ];
int IMX[N_FREQ];
uint16_t MAG[N_FREQ];
uint8_t lcd_buf_top[N_FREQ];
uint8_t lcd_buf_bot[N_FREQ];

void init_adc(void);
void init_timer(void);
void init_pwm(void);
void init_lcd(void);
uint16_t read_adc(void);
void write_pwm(uint16_t val);
void lcd_update(void);
void dft(void);
void mag_to_buf(void);

void TIM3_IRQHandler()
{
	// Checks whether the TIM3 interrupt has occurred or not
	if (TIM_GetITStatus(TIM3, TIM_IT_Update))
	{
		// Read ADC value (10-bit PWM)
		adc_value = read_adc() >> 2;
		
		// Write to PWM (audio loopback)
		write_pwm(adc_value);
		
		// Sampling N_TIME point DFT
		if (n_done == 0)
		{
			x[n_count++] = adc_value;
			
			if (n_count >= N_TIME)
			{
				n_done = 1;
				n_count = 0;			
			}
		}
		
		// Clears the TIM3 interrupt pending bit
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}

int main(void)
{
	init_adc();
	init_timer();
	init_pwm();
	init_lcd();
	
	while (1)
	{
		// Wait until sampling is done
		while (!n_done);
		dft();
		mag_to_buf();
		lcd_update();
		n_done = 0;
	}
}

void init_adc()
{
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
	// ADC1 channel 1 (PA1)
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_7Cycles5);
	
	// Step 2: Initialize GPIOA (PA1) for analog input
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void init_timer()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	// Step 1: Initialize TIM3 for timer interrupt
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	// Timer freq = timer_clock / ((TIM_Prescaler+1) * (TIM_Period+1))
	// Timer freq = 72MHz / ((1+1) * (1023+1) = 35.15kHz
	TIM_TimeBaseInitStruct.TIM_Prescaler = 1;
	TIM_TimeBaseInitStruct.TIM_Period = 1023;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	// Enable TIM3 interrupt
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

	// Step 2: Initialize NVIC for timer interrupt
	NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

void init_pwm()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Step 1: Initialize TIM2 for PWM
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	// Timer freq = timer_clock / ((TIM_Prescaler+1) * (TIM_Period+1))
	// Timer freq = 72MHz / ((1+1) * (1023+1) = 35.15kHz
	TIM_TimeBaseInitStruct.TIM_Prescaler = 1;
	TIM_TimeBaseInitStruct.TIM_Period = 1023;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	TIM_Cmd(TIM2, ENABLE);
	
	// Step 2: Initialize PWM
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_Pulse = 0;
	TIM_OC1Init(TIM2, &TIM_OCInitStruct);
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
	
	// Step 3: Initialize GPIOA (PA0) for PWM output
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void init_lcd()
{
	uint8_t i;
	
	// Initialize LCD
	lcd16x2_init(LCD16X2_DISPLAY_ON_CURSOR_OFF_BLINK_OFF);
	
	// Fill custom char
	for (i = 0; i < 8; i++)
	{
		lcd16x2_create_custom_char(i, bar_graph[i]);
	}
}

uint16_t read_adc()
{
	// Start ADC conversion
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	// Wait until ADC conversion finished
	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

	return ADC_GetConversionValue(ADC1);
}

void write_pwm(uint16_t val)
{
	// Write PWM value
	TIM2->CCR1 = val;
}

void lcd_update()
{
	uint8_t i;
	
	// Write 16 point frequency spectrum (index 1 to 16)
	// Frequency spectrum at index 0 (DC value) is not used
	for (i = 1; i < N_FREQ; i++)
	{
		// Write first row
		if (lcd_buf_top[i] == ' ')
		{
			lcd16x2_gotoxy((i-1), 0);
			lcd16x2_putc(' ');
		}
		else
		{
			lcd16x2_put_custom_char((i-1), 0, lcd_buf_top[i]);
		}
		// Write second row
		lcd16x2_put_custom_char((i-1), 1, lcd_buf_bot[i]);
	}
}

void dft()
{
	uint8_t k, i;
	uint16_t lookup_idx = 0;
	
	// Zero REX[] and IMX[] so they can be used as accumulators
	for (k = 0; k < N_FREQ; k++)
	{
		REX[k] = 0;
		IMX[k] = 0;
	}
	
	// Loop through each sample in the frequency domain
	for (k = 0; k < N_FREQ; k++)
	{
		// Loop through each sample in the time domain
		for (i = 0; i < N_TIME; i++)
		{
			REX[k] += x[i] * cos_lookup[lookup_idx];
			IMX[k] += -x[i] * sin_lookup[lookup_idx];
			lookup_idx++;
		}
		
		// Calculate magnitude from real and imaginary part
		MAG[k] = sqrt(REX[k]*REX[k] + IMX[k]*IMX[k]);
	}
}

void mag_to_buf()
{
	uint8_t i;
	
	// Convert magnitude to bar graph display on LCD
	for(i = 1; i < N_FREQ; i++) 
	{
		// Scaling magnitude to fit the LCD bar graph maximum value
		MAG[i] /= 32;
		
		// Fill LCD row buffer
		if (MAG[i] > 15)
		{
			lcd_buf_top[i] = 7;
			lcd_buf_bot[i] = 7;
		}
		else if (MAG[i] > 7)
		{
			lcd_buf_top[i] = MAG[i] - 7 - 1;
			lcd_buf_bot[i] = 7;
		}
		else
		{
			lcd_buf_top[i] = ' ';
			lcd_buf_bot[i] = MAG[i];
		}
	}
}
