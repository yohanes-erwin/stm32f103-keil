#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "delay.h"

void EXTI15_10_Init(void);
void wait(unsigned int nCount);

void EXTI15_10_IRQHandler(void)
{
	// Checks whether the interrupt is from EXTI12 or not
	if (EXTI_GetITStatus(EXTI_Line12))
	{
		// Blink LED on PB13 5 times
		int i;
		for (i = 0; i < 5; i++)
		{
			GPIOB->BSRR = GPIO_Pin_13;
			wait(100);
			GPIOB->BRR = GPIO_Pin_13;
			wait(100);
		}
		
		// Clear pending bit
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
}

GPIO_InitTypeDef GPIO_InitStruct;

int main(void)
{
	DelayInit();
	
	// Initialize exernal interrupt
	EXTI15_10_Init();
	
	// Initialize PB13 as push-pull output for LED
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	// Initialize PC13 as push-pull output for LED
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	while (1)
	{
		// Blink LED on PC13
		GPIOC->BRR = GPIO_Pin_13;
		DelayMs(1000);
		GPIOC->BSRR = GPIO_Pin_13;
		DelayMs(1000);
	}
}

void EXTI15_10_Init()
{
	// Initalization struct
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	// Initialize PB12 as input with pull-up resistor
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// Initialize PB12 to EXTI
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
	
	// Initialize EXTI line 12 for PB12
	EXTI_InitStruct.EXTI_Line = EXTI_Line12;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
	// Initialize NVIC for EXTI15_10 IRQ channel
	NVIC_InitStruct.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

// Wait function
void wait(unsigned int nCount)
{
	unsigned int i, j;
	
	for (i = 0; i < nCount; i++)
		for (j = 0; j < 0x2AFF; j++);
}
