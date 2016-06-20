#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "delay.h"

#define SS_DIGIT				4
#define RCC_GPIO_DIG		RCC_APB2Periph_GPIOA
#define RCC_GPIO_SEG		RCC_APB2Periph_GPIOB
#define GPIO_DIG				GPIOA
#define GPIO_SEG				GPIOB
#define GPIO_PIN_DIG_0	GPIO_Pin_11
#define GPIO_PIN_DIG_1	GPIO_Pin_10
#define GPIO_PIN_DIG_2	GPIO_Pin_9
#define GPIO_PIN_DIG_3	GPIO_Pin_8
#define GPIO_PIN_SEG_A	GPIO_Pin_14
#define GPIO_PIN_SEG_B	GPIO_Pin_13
#define GPIO_PIN_SEG_C	GPIO_Pin_12
#define GPIO_PIN_SEG_D	GPIO_Pin_11
#define GPIO_PIN_SEG_E	GPIO_Pin_10
#define GPIO_PIN_SEG_F	GPIO_Pin_9
#define GPIO_PIN_SEG_G	GPIO_Pin_8

// 7 segment font (0-9)
// D7=DP, D6=A, D5=B, D4=C, D3=D, D2=E, D1=F, D0=G
const uint8_t font[10] =
{
	0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B
};
uint8_t buffer[4];

void TIM2_INT_Init(void);
void Led7SegmentInit(void);
void ScreenUpdate(void);
void PrintNumber(uint16_t number);

void TIM2_IRQHandler()
{
	// Checks whether the TIM2 interrupt has occurred or not
	if (TIM_GetITStatus(TIM2, TIM_IT_Update))
	{
		// Scan 1 digit every 1ms timer interrupt
		ScreenUpdate();

		// Clears the TIM2 interrupt pending bit
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

int main(void)
{
	DelayInit();
	TIM2_INT_Init();
	Led7SegmentInit();
	
	while (1)
	{
		PrintNumber(505);
		DelayMs(1000);
		PrintNumber(2016);
		DelayMs(1000);
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

void Led7SegmentInit()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Initialize GPIO for LED digit
	RCC_APB2PeriphClockCmd(RCC_GPIO_DIG, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_PIN_DIG_0 | GPIO_PIN_DIG_1 | GPIO_PIN_DIG_2 |
		GPIO_PIN_DIG_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIO_DIG, &GPIO_InitStruct);
	
	// Initialize GPIO for LED segment
	RCC_APB2PeriphClockCmd(RCC_GPIO_SEG, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_PIN_SEG_A | GPIO_PIN_SEG_B | GPIO_PIN_SEG_C |
		GPIO_PIN_SEG_D | GPIO_PIN_SEG_E | GPIO_PIN_SEG_F | GPIO_PIN_SEG_G;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIO_SEG, &GPIO_InitStruct);
}

void ScreenUpdate()
{
	static uint8_t digit = 0;
	
	// Digit scanning
	// Digit pins is connected to the base of pnp transistor
	GPIO_DIG->ODR = ~(1 << (digit + 8)) & 0x0F00;
	// The LED 7 segment is common anode
	GPIO_SEG->ODR = (~font[buffer[digit]] << 8) & 0xFF00;
	
	digit++;
	if (digit > (SS_DIGIT-1))
	{
		digit = 0;
	}
}

void PrintNumber(uint16_t number)
{
	// Check max and min
	if (number > 9999)
	{
		number = 9999;
	}
	
	// Convert integer to bcd digits
	buffer[0] = number / 1000;
	buffer[1] = number % 1000 / 100;
	buffer[2] = number % 100 / 10;
	buffer[3] = number % 10;
}
