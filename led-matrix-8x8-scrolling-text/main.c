#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "delay.h"
#include "font.h"

#define RCC_GPIO_ROW		RCC_APB2Periph_GPIOA
#define RCC_GPIO_COL		RCC_APB2Periph_GPIOB
#define GPIO_ROW				GPIOA
#define GPIO_COL				GPIOB
#define GPIO_PIN_ROW_0	GPIO_Pin_7
#define GPIO_PIN_ROW_1	GPIO_Pin_6
#define GPIO_PIN_ROW_2	GPIO_Pin_5
#define GPIO_PIN_ROW_3	GPIO_Pin_4
#define GPIO_PIN_ROW_4	GPIO_Pin_3
#define GPIO_PIN_ROW_5	GPIO_Pin_2
#define GPIO_PIN_ROW_6	GPIO_Pin_1
#define GPIO_PIN_ROW_7	GPIO_Pin_0
#define GPIO_PIN_COL_0	GPIO_Pin_15
#define GPIO_PIN_COL_1	GPIO_Pin_14
#define GPIO_PIN_COL_2	GPIO_Pin_13
#define GPIO_PIN_COL_3	GPIO_Pin_12
#define GPIO_PIN_COL_4	GPIO_Pin_11
#define GPIO_PIN_COL_5	GPIO_Pin_10
#define GPIO_PIN_COL_6	GPIO_Pin_9
#define GPIO_PIN_COL_7	GPIO_Pin_8

uint8_t buffer[8];

void TIM2_INT_Init(void);
void LedMatrixInit(void);
void ScreenUpdate(void);
void Scroll(char text[]);

void TIM2_IRQHandler()
{
	// Checks whether the TIM2 interrupt has occurred or not
	if (TIM_GetITStatus(TIM2, TIM_IT_Update))
	{
		// Scan 1 row every 1ms timer interrupt
		ScreenUpdate();

		// Clears the TIM2 interrupt pending bit
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

int main(void)
{
	DelayInit();
	
	// Initialize timer interrupt
	TIM2_INT_Init();
	// Initialize LED matrix GPIO pin
	LedMatrixInit();
	
	while (1)
	{
		Scroll("  8x8 LED Matrix Scrolling Text  ");
	}
}

void TIM2_INT_Init()
{
	// Init struct
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	// Enable clock for TIM2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	// TIM2 initialization for overflow every 1ms
	// Update Event (Hz) = timer_clock / ((TIM_Prescaler + 1) * (TIM_Period + 1))
	// Update Event (Hz) = 72MHz / ((71 + 1) * (999 + 1)) = 1000Hz (1ms)
	TIM_TimeBaseInitStruct.TIM_Prescaler = 71;
	TIM_TimeBaseInitStruct.TIM_Period = 999;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	
	// Enable TIM2 interrupt
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	// Start TIM2
	TIM_Cmd(TIM2, ENABLE);
	
	// Nested vectored interrupt settings
	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

void LedMatrixInit()
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

void ScreenUpdate()
{
	static uint8_t row = 0;
	
	// Scan every row pin with high logic, because row pins is anode
	GPIO_ROW->ODR = (1 << row) & 0x00FF;
	// Set column value for every row
	// Buffer value is inverted because column pins is cathode
	GPIO_COL->ODR = (~buffer[row] << 8) & 0xFF00;
	
	row++;
	if (row > 7)
	{
		row = 0;
	}
}

void Scroll(char text[])
{
	uint8_t txt_length = 0;
	uint8_t chr_index = 0;
	uint8_t chr;
	uint8_t chr_row;
	uint8_t scroll;
	uint8_t row;
	
	// Count text length
	while (text[txt_length])
	{
		txt_length++;
	}
	
	// Scroll all chars in text
	while (chr_index < (txt_length-1))
	{
		chr = text[chr_index];
		
		for (scroll = 0; scroll <= 7; scroll++)
		{
			for (row = 0; row <= 7; row++)
			{
				chr_row = font5x7[chr - 32][row];
				buffer[row] = (buffer[row] << 1) | (chr_row >> (7-scroll));
			}
			DelayMs(75);
		}
		
		chr_index++;
	}
}
