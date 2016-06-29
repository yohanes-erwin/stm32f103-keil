#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "delay.h"
#include "lcd16x2.h"
#include <stdio.h>

// LCD custom char
uint8_t bar[][8] = 
{ 
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 },
	{ 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 },
	{ 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C },
	{ 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E },
	{ 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F }
};
volatile uint8_t last_enc_val_a;
volatile uint8_t last_enc_val_b;
volatile int enc_cnt;
char enc_cnt_buf[8];

void init_lcd(void);
void init_rotary_encoder(void);
void lcd_update(void);
void rotary_encoder_update(void);

void EXTI9_5_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line6) | EXTI_GetITStatus(EXTI_Line5))
	{
		rotary_encoder_update();
		
		// Clear interrupt flag
		EXTI_ClearITPendingBit(EXTI_Line6);
		EXTI_ClearITPendingBit(EXTI_Line5);
	}
}

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
	uint8_t i;
	
	// Initialize LCD
	lcd16x2_init(LCD16X2_DISPLAY_ON_CURSOR_OFF_BLINK_OFF);
	
	// Fill custom char
	for (i = 0; i < 8; i++)
	{
		lcd16x2_create_custom_char(i, bar[i]);
	}
}

void init_rotary_encoder()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	// Step 1: Initialize GPIO as input for rotary encoder
	// PB6 (encoder pin A), PB5 (encoder pin B)
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// Step 2: Initialize EXTI for PB6 and PB5
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);
	EXTI_InitStruct.EXTI_Line = EXTI_Line6 | EXTI_Line5;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
	// Step 3: Initialize NVIC for EXTI9_5 IRQ channel
	NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

void lcd_update()
{
	uint8_t div_bar, mod_bar;
	uint8_t i;
	
	div_bar = enc_cnt / 5;
	mod_bar = enc_cnt % 5;
	sprintf(enc_cnt_buf, "%i", enc_cnt);
	
	lcd16x2_clrscr();
	for (i = 0; i < div_bar; i++)
	{
		lcd16x2_put_custom_char(i, 0, 5);
	}
	lcd16x2_put_custom_char(i, 0, mod_bar);
	lcd16x2_gotoxy(0, 1);
	lcd16x2_puts(enc_cnt_buf);
	
	DelayMs(250);
}

void rotary_encoder_update()
{
	uint8_t enc_val_a, enc_val_b;
	uint8_t enc_inc, enc_dec;
	
	// Read pin A and pin B every rising and falling edge interrupt
	// from both pin (4x resolution)
	enc_val_a = (uint8_t) ((GPIOB->IDR & GPIO_Pin_6) >> 6);
	enc_val_b = (uint8_t) ((GPIOB->IDR & GPIO_Pin_5) >> 5);
	
	// Read encoder direction using xor logic
	enc_inc = enc_val_a ^ last_enc_val_b;
	enc_dec = enc_val_b ^ last_enc_val_a;
	
	// Decrement or increment counter
	if(enc_inc)
	{
		enc_cnt++;
	}
	if(enc_dec)
	{
		enc_cnt--;
	}
	
	// Store encoder value for next reading
	last_enc_val_a = enc_val_a;
	last_enc_val_b = enc_val_b;
	
	// Set max and min value
	if (enc_cnt > 80)
	{
		enc_cnt = 80;
	}
	if (enc_cnt < 0)
	{
		enc_cnt = 0;
	}
}
