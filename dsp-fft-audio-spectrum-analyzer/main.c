/**
  ******************************************************************************
	* @file		main.c
  * @author	Yohanes Erwin Setiawan
  * @date		21 Jun 2016 
	******************************************************************************
	* @brief	Simple spectrum analyzer using 16 point FFT
	*					Audio loopback (Read ADC value then write back to PWM)
	* 				1. ADC: 
	*							- ADC1 channel 1 (PA1) (10-bit)
	*					2. TIMER:
	*							- TIM3
	*							- Interrupt rate = 35.15kHz
	*							- Audio sampling rate = 35.15kHz
	*							- FFT sampling rate = 17.5kHz
	*							- LED matrix scanning rate = 1kHz
	*					3. PWM:
	*							- TIM2
	*							- PWM frequency = 35.15kHz 
	*							- PWM pin PA0 (10-bit)
	*					4. DISPLAY:
	*							- LED matrix 8x8
	*					5. FFT:
	*							- Length = 16 point
	*							- Nyquist frequency =  17.5kHz/2 = 8.75kHz
	*							- Bandwidth = 8.75kHz/8 = 1.1kHz
	******************************************************************************
	*/

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_tim.h"
#include <math.h>

// 16 point FFT
#define N		16
#define PI	3.141

#define RCC_GPIO_ROW		RCC_APB2Periph_GPIOA
#define RCC_GPIO_COL		RCC_APB2Periph_GPIOB
#define GPIO_ROW				GPIOA
#define GPIO_COL				GPIOB
#define GPIO_PIN_ROW_0	GPIO_Pin_4
#define GPIO_PIN_ROW_1	GPIO_Pin_5
#define GPIO_PIN_ROW_2	GPIO_Pin_6
#define GPIO_PIN_ROW_3	GPIO_Pin_7
#define GPIO_PIN_ROW_4	GPIO_Pin_8
#define GPIO_PIN_ROW_5	GPIO_Pin_9
#define GPIO_PIN_ROW_6	GPIO_Pin_10
#define GPIO_PIN_ROW_7	GPIO_Pin_11
#define GPIO_PIN_COL_0	GPIO_Pin_8
#define GPIO_PIN_COL_1	GPIO_Pin_9
#define GPIO_PIN_COL_2	GPIO_Pin_10
#define GPIO_PIN_COL_3	GPIO_Pin_11
#define GPIO_PIN_COL_4	GPIO_Pin_12
#define GPIO_PIN_COL_5	GPIO_Pin_13
#define GPIO_PIN_COL_6	GPIO_Pin_14
#define GPIO_PIN_COL_7	GPIO_Pin_15

volatile uint16_t adc_value = 0;
volatile uint8_t n_count = 0;
volatile uint8_t n_done = 0;
int REX[N];
int IMX[N];
uint16_t MAG[N];
uint8_t led_buf[8];

void init_adc(void);
void init_timer(void);
void init_pwm(void);
void init_led_matrix(void);
uint16_t read_adc(void);
void write_pwm(uint16_t val);
void led_matrix_update(void);
void fft(void);
void mag_to_buf(void);

void TIM3_IRQHandler()
{
	static uint8_t s = 0;
	static uint8_t l = 0;
	
	// TIM3 interrupt at 35.15kHz
	if (TIM_GetITStatus(TIM3, TIM_IT_Update))
	{
		// Read ADC value (10-bit PWM)
		adc_value = read_adc() >> 2;
		// Write to PWM (audio loopback)
		write_pwm(adc_value);
		
		// Sampling N point FFT at 17.5kHz
		s++;
		if (s >= 2)
		{
			if (n_done == 0)
			{
				REX[n_count++] = adc_value;
				
				if (n_count >= N)
				{
					n_done = 1;
					n_count = 0;			
				}
			}
			s = 0;
		}
		
		// LED matrix scanning at 1kHz
		l++;
		if (l >= 35)
		{
			led_matrix_update();
			l = 0;
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
	init_led_matrix();
	
	while (1)
	{
		// Wait until sampling is done
		while (!n_done);
		fft();
		mag_to_buf();
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

void init_led_matrix()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Initialize GPIO for LED matrix rows
	RCC_APB2PeriphClockCmd(RCC_GPIO_ROW, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_PIN_ROW_0 | GPIO_PIN_ROW_1 | GPIO_PIN_ROW_2 |
		GPIO_PIN_ROW_3 | GPIO_PIN_ROW_4 | GPIO_PIN_ROW_5 | GPIO_PIN_ROW_6 |
		GPIO_PIN_ROW_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIO_ROW, &GPIO_InitStruct);
	
	// Initialize GPIO for LED matrix columns
	RCC_APB2PeriphClockCmd(RCC_GPIO_COL, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_PIN_COL_0 | GPIO_PIN_COL_1 | GPIO_PIN_COL_2 |
		GPIO_PIN_COL_3 | GPIO_PIN_COL_4 | GPIO_PIN_COL_5 | GPIO_PIN_COL_6 |
		GPIO_PIN_COL_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIO_COL, &GPIO_InitStruct);
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

void led_matrix_update()
{
	static uint8_t row = 0;
	
	// Scan every row pin with high logic, because row pins is anode
	// (row+4) because row pin start from PA4
	GPIO_ROW->ODR = (1 << (row+4)) & 0x0FF0;
	// Set column value for every row
	// Buffer value is inverted because column pins is cathode
	GPIO_COL->ODR = (~led_buf[7-row] << 8) & 0xFF00;
	
	row++;
	if (row > 7)
	{
		row = 0;
	}
}

void fft()
{
	uint8_t nm1 = N - 1;
	uint8_t nd2 = N / 2;
	uint8_t m = log10(N)/log10(2);
	uint8_t j = nd2;
	uint8_t i, k, l;
	uint8_t le, le2;
	float ur, ui, sr, si;
	uint8_t jm1, ip;
	float tr, ti;
	
	// Set imaginary part input signal to 0
	for (i = 0; i <= nm1; i++)
	{
		IMX[i] = 0;
	}
	
	// Bit reversal sorting
	for (i = 1; i <= N-2; i++)
	{
		if (i >= j) goto a;
		tr = REX[j];
		ti = IMX[j];
		REX[j] = REX[i];
		IMX[j] = IMX[i];
		REX[i] = tr;
		IMX[i] = ti;
		a:
		k = nd2;
		b:
		if (k > j) goto c;
		j -= k;
		k /= 2;
		goto b;
		c:
		j += k;
	}

	// Loop for each FFT stage
	for (l = 1; l <= m; l++)
	{
		le = pow(2, l);
		le2 = le / 2;
		ur = 1;
		ui = 0;
		// Calculate sine and cosine values
		sr = cos(PI/le2);
		si = -sin(PI/le2);
		// Loop for each sub FFT
		for (j = 1; j <= le2; j++)
		{
			jm1 = j - 1;
			// Loop for each butterfly
			for (i = jm1; i <= nm1; i += le)
			{
				ip = i + le2;
				tr = REX[ip]*ur - IMX[ip]*ui;
				ti = REX[ip]*ui + IMX[ip]*ur;
				REX[ip] = REX[i] - tr;
				IMX[ip] = IMX[i] - ti;
				REX[i] = REX[i] + tr;
				IMX[i] = IMX[i] + ti;
			}
			tr = ur;
			ur = tr*sr - ui*si;
			ui = tr*si + ui*sr;
		}
	}
}

void mag_to_buf()
{
	uint8_t i, j;
	
	// Loop for each magnitude
	for (i = 0; i <= N/2; i++)
	{
		// Calculate magnitude
		MAG[i] = sqrt(REX[i]*REX[i] + IMX[i]*IMX[i]);
		
		// Scaling magnitude
		MAG[i] /= 64;
		if (MAG[i] > 8)
		{
			MAG[i] = 8;
		}
	}
	
	// Loop for each column
	for(i = 0; i <= 7; i++) 
	{
		// Loop for each row
		for (j = 0; j <= 7; j++)
		{
			// Clear magnitude value for column i
			led_buf[j] &= ~(1 << (i)); 		
		}
		// Loop for each row
		// Display 8 point frequency spectrum (MAG) from index 1 to 8
		// Frequency spectrum at index 0 (DC value) is not displayed
		// Frequency spectrum from index 9 to 15 is not displayed
		for (j = 0; j < MAG[i+1]; j++)
		{
			// Set magnitude value for column i
			led_buf[j] |= (1 << (i)); 		
		}
	}
}
