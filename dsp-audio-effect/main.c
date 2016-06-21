#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_tim.h"
#include "delay.h"

#define LOW_PASS				0x1000
#define PITCH_UP				0x2000
#define PITCH_DOWN			0x4000

#define FILTER_BUF   		9
#define PITCH_BUF   		500

/* Low pass filter coefficient (frequency cutoff = 800Hz)
 * Matlab code:
 *	N = 7; f_lp = 800; fs = 35156;
 *	Wn = f_lp/(fs/2);
 *	B = fir1(N, Wn, 'low');
 *	freqz(B);
 */
const float filter_coeff[FILTER_BUF] = 
{
	0.0200, 0.0647, 0.1664, 0.2489, 0.2489, 0.1664, 0.0647, 0.0200
};
volatile uint16_t adcValue = 0;
volatile uint16_t effect = 0;

void ADC_Setup(void);
void PWM_Setup(void);
void GPIO_Setup(void);
uint16_t ADC_Read(void);
void PWM_Write(uint16_t val);
uint16_t low_pass(uint16_t input);
uint16_t pitch_up(uint16_t input);
uint16_t pitch_down(uint16_t input);

void TIM3_IRQHandler()
{
	// Checks whether the TIM3 interrupt has occurred or not
	if (TIM_GetITStatus(TIM3, TIM_IT_Update))
	{
		// Read ADC value (10-bit PWM)
		adcValue = ADC_Read() >> 2;
		
		// Add audio effect
		if (effect & LOW_PASS)
		{
			adcValue = low_pass(adcValue);
		}
		if (effect & PITCH_UP)
		{
			adcValue = pitch_up(adcValue);
		}
		if (effect & PITCH_DOWN)
		{
			adcValue = pitch_down(adcValue);
		}
		
		// Write to PWM
		PWM_Write(adcValue);
		
		// Clears the TIM3 interrupt pending bit
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}

int main(void)
{
	// Initialize delay function
	DelayInit();
	
	// Initialize ADC, PWM, and GPIO
	ADC_Setup();
	PWM_Setup();
	GPIO_Setup();
	
	while (1)
	{
		// Read input switch (active low)
		effect = GPIO_ReadInputData(GPIOB);
		// Invert and mask input switch bits
		effect = ~effect & 0x7000;
		
		// If any audio effect is active, then turn on LED 
		if (effect)
		{
			// Turn on LED (active low)
			GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		}
		else
		{
			GPIO_SetBits(GPIOC, GPIO_Pin_13);
		}
		
		DelayMs(50);
	}
}

void ADC_Setup()
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

void PWM_Setup()
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
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
	
	// Step 3: Initialize TIM3 for timer interrupt
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

	// Step 4: Initialize NVIC for timer interrupt
	NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	// Step 5: Initialize GPIOA (PA0) for PWM output
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void GPIO_Setup()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Initialize GPIOC (PC13) for LED
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	// Initialize GPIOB (PB12, PB13, PB14, PB15) for switch
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | 
		GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
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

uint16_t low_pass(uint16_t input)
{
	int i;
	static float buffer[FILTER_BUF];
	uint16_t result;
	
  for (i = (FILTER_BUF-1); i > 0; i--)
	{
		buffer[i] = buffer[i-1];
	}
	buffer[0] = input;
	
  for (i = 0; i < FILTER_BUF; i++)
  {
		result += buffer[i] * filter_coeff[i];
  }
	
	return result;
}

uint16_t pitch_up(uint16_t input)
{
	static int index_wr = 0;
	static int index_rd = 1;
	static uint16_t buffer[PITCH_BUF];
	
	buffer[index_wr] = input;
	
	index_rd += 2;
	if (index_rd >= (PITCH_BUF-1))
	{
		index_rd = 0;
	}
	index_wr++;
	if (index_wr >= (PITCH_BUF-1))
	{
		index_wr = 0;
	}
	
	return buffer[index_rd];
}

uint16_t pitch_down(uint16_t input)
{
	static int index_wr = 0;
	static int index_rd = 1;
	static uint8_t half = 0;
	static uint16_t buffer[PITCH_BUF];
	
	buffer[index_wr] = input;
	
	half++;
	if (half == 2)
	{
		index_rd++;
		half = 0;
	}
	if (index_rd >= (PITCH_BUF-1))
	{
		index_rd = 0;
	}
	index_wr++;
	if (index_wr >= (PITCH_BUF-1))
	{
		index_wr = 0;
	}
	
	return buffer[index_rd];
}
