#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "delay.h"
#include "lcd16x2.h"
#include <stdio.h>

uint16_t enc_cnt;
char enc_cnt_buf[8];

void init_lcd(void);
void init_rotary_encoder(void);
void lcd_update(void);

int main(void)
{
	DelayInit();
	init_lcd();
	init_rotary_encoder();
	
	while (1)
	{
		lcd_update();
	}
}

void init_lcd()
{
	// Initialize LCD
	lcd16x2_init(LCD16X2_DISPLAY_ON_CURSOR_OFF_BLINK_OFF);
}

void init_rotary_encoder()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Step 1: Initialize GPIO as input for rotary encoder
	// PB7 (TIM4_CH2) (encoder pin A), PB6 (TIM4_CH1) (encoder pin B)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// Step 2: Setup TIM4 for encoder input
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, 
		TIM_ICPolarity_Falling);
	TIM_Cmd(TIM4, ENABLE);
}

void lcd_update()
{
	// Get encoder value
	enc_cnt = TIM_GetCounter(TIM4);
	
	// Print encoder value
	sprintf(enc_cnt_buf, "%i", enc_cnt);
	lcd16x2_clrscr();
	lcd16x2_puts(enc_cnt_buf);
	
	DelayMs(250);
}
