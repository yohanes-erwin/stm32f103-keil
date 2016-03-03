/**
  ******************************************************************************
  * @file		keypad4x4-simultaneous-scanning.h
  * @author	Yohanes Erwin Setiawan
  * @date		3 March 2016
  ******************************************************************************
  */

#ifndef __KEYPAD4X4_SIMULTANEOUS_H
#define __KEYPAD4X4_SIMULTANEOUS_H

#ifdef __cplusplus
extern "C" {
#endif

/** Includes ---------------------------------------------------------------- */
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "delay.h"	

// GPIO pin definitions for keypad columns (must on the same GPIO)
#define KEYPAD_RCC_GPIO_COL		RCC_APB2Periph_GPIOA
#define KEYPAD_GPIO_COL				GPIOA
#define KEYPAD_PIN_COL0				GPIO_Pin_7
#define KEYPAD_PIN_COL1				GPIO_Pin_6
#define KEYPAD_PIN_COL2				GPIO_Pin_5
#define KEYPAD_PIN_COL3				GPIO_Pin_4
// GPIO pin definitions for keypad rows (must on the same GPIO)
#define KEYPAD_RCC_GPIO_ROW		RCC_APB2Periph_GPIOA
#define KEYPAD_GPIO_ROW				GPIOA
#define KEYPAD_PIN_ROW0				GPIO_Pin_3
#define KEYPAD_PIN_ROW1				GPIO_Pin_2
#define KEYPAD_PIN_ROW2				GPIO_Pin_1
#define KEYPAD_PIN_ROW3				GPIO_Pin_0
// Return value for key pressed
#define KEYPAD_NO_PRESSED			0xFF
#define KEYPAD_ROW0_COL0			'1'
#define KEYPAD_ROW0_COL1			'2'
#define KEYPAD_ROW0_COL2			'3'
#define KEYPAD_ROW0_COL3			'A'
#define KEYPAD_ROW1_COL0			'4'
#define KEYPAD_ROW1_COL1			'5'
#define KEYPAD_ROW1_COL2			'6'
#define KEYPAD_ROW1_COL3			'B'
#define KEYPAD_ROW2_COL0			'7'
#define KEYPAD_ROW2_COL1			'8'
#define KEYPAD_ROW2_COL2			'9'
#define KEYPAD_ROW2_COL3			'C'
#define KEYPAD_ROW3_COL0			'*'
#define KEYPAD_ROW3_COL1			'0'
#define KEYPAD_ROW3_COL2			'#'
#define KEYPAD_ROW3_COL3			'D'

/** Public function prototypes ---------------------------------------------- */
void KeypadInit(void);
uint8_t KeypadGetKey(void);

#ifdef __cplusplus
}
#endif

#endif

/********************************* END OF FILE ********************************/
/******************************************************************************/
