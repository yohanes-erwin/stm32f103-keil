/**
  ******************************************************************************
  * @file		keypad4x4-simultaneous-scanning.c
  * @author	Yohanes Erwin Setiawan
  * @date		3 March 2016
  ******************************************************************************
  */

/** Includes ---------------------------------------------------------------- */
#include "keypad4x4-simultaneous-scanning.h"

GPIO_InitTypeDef _GPIO_InitStructKeypad;

/** Public functions -------------------------------------------------------- */
/**
  ******************************************************************************
  * @brief	Initialize GPIO pins for keypad.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void KeypadInit()
{
	DelayInit();
	
	// GPIO clock for keypad columns and rows
	RCC_APB2PeriphClockCmd(KEYPAD_RCC_GPIO_COL, ENABLE);
	RCC_APB2PeriphClockCmd(KEYPAD_RCC_GPIO_ROW, ENABLE);
}

/**
  ******************************************************************************
  * @brief	Get which key is pressed by scanning the columns and read the rows.
  * @param	None
  * @retval	Pressed key char value.
  ******************************************************************************
  */
uint8_t KeypadGetKey()
{
	// For store keypad column and row bits value
	// Column bits stored in high nibble, row bits stored in low nibble
	// cols: 0b C0C1C2C3 0000
	// rows: 0b 0000 R0R1R2R3
	uint8_t cols = 0, rows = 0;
		
	/* Drive all columns with logic high, then read all rows value */
	// Configure GPIO as output push-pull for keypad columns
	_GPIO_InitStructKeypad.GPIO_Pin = KEYPAD_PIN_COL0 | KEYPAD_PIN_COL1 |
		KEYPAD_PIN_COL2 | KEYPAD_PIN_COL3;
	_GPIO_InitStructKeypad.GPIO_Mode = GPIO_Mode_Out_PP;
	_GPIO_InitStructKeypad.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(KEYPAD_GPIO_COL, &_GPIO_InitStructKeypad);
	// Configure GPIO as input with pull-down resistor for keypad rows
	_GPIO_InitStructKeypad.GPIO_Pin = KEYPAD_PIN_ROW0 | KEYPAD_PIN_ROW1 |
		KEYPAD_PIN_ROW2 | KEYPAD_PIN_ROW3;
	_GPIO_InitStructKeypad.GPIO_Mode = GPIO_Mode_IPD;
	_GPIO_InitStructKeypad.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(KEYPAD_GPIO_ROW, &_GPIO_InitStructKeypad);
	// Drive all columns
	GPIO_SetBits(KEYPAD_GPIO_COL, KEYPAD_PIN_COL0);
	GPIO_SetBits(KEYPAD_GPIO_COL, KEYPAD_PIN_COL1);
	GPIO_SetBits(KEYPAD_GPIO_COL, KEYPAD_PIN_COL2);
	GPIO_SetBits(KEYPAD_GPIO_COL, KEYPAD_PIN_COL3);
	DelayUs(1);
	// Read all rows
	if (GPIO_ReadInputDataBit(KEYPAD_GPIO_ROW, KEYPAD_PIN_ROW0))
		rows |= (1 << 3);
	if (GPIO_ReadInputDataBit(KEYPAD_GPIO_ROW, KEYPAD_PIN_ROW1))
		rows |= (1 << 2);
	if (GPIO_ReadInputDataBit(KEYPAD_GPIO_ROW, KEYPAD_PIN_ROW2))
		rows |= (1 << 1);
	if (GPIO_ReadInputDataBit(KEYPAD_GPIO_ROW, KEYPAD_PIN_ROW3))
		rows |= (1 << 0);
	
	/* Drive all rows with logic high, then read all columns value */
	// Configure GPIO as output push-pull for keypad rows
	_GPIO_InitStructKeypad.GPIO_Pin = KEYPAD_PIN_ROW0 | KEYPAD_PIN_ROW1 |
		KEYPAD_PIN_ROW2 | KEYPAD_PIN_ROW3;
	_GPIO_InitStructKeypad.GPIO_Mode = GPIO_Mode_Out_PP;
	_GPIO_InitStructKeypad.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(KEYPAD_GPIO_ROW, &_GPIO_InitStructKeypad);
	// Configure GPIO as input with pull-down resistor for keypad columns
	_GPIO_InitStructKeypad.GPIO_Pin = KEYPAD_PIN_COL0 | KEYPAD_PIN_COL1 |
		KEYPAD_PIN_COL2 | KEYPAD_PIN_COL3;
	_GPIO_InitStructKeypad.GPIO_Mode = GPIO_Mode_IPD;
	_GPIO_InitStructKeypad.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(KEYPAD_GPIO_COL, &_GPIO_InitStructKeypad);	
	// Drive all rows
	GPIO_SetBits(KEYPAD_GPIO_ROW, KEYPAD_PIN_ROW0);
	GPIO_SetBits(KEYPAD_GPIO_ROW, KEYPAD_PIN_ROW1);
	GPIO_SetBits(KEYPAD_GPIO_ROW, KEYPAD_PIN_ROW2);
	GPIO_SetBits(KEYPAD_GPIO_ROW, KEYPAD_PIN_ROW3);
	DelayUs(1);
	// Read all columns
	if (GPIO_ReadInputDataBit(KEYPAD_GPIO_COL, KEYPAD_PIN_COL0))
		cols |= (1 << 7);
	if (GPIO_ReadInputDataBit(KEYPAD_GPIO_COL, KEYPAD_PIN_COL1))
		cols |= (1 << 6);
	if (GPIO_ReadInputDataBit(KEYPAD_GPIO_COL, KEYPAD_PIN_COL2))
		cols |= (1 << 5);
	if (GPIO_ReadInputDataBit(KEYPAD_GPIO_COL, KEYPAD_PIN_COL3))
		cols |= (1 << 4);
	
	/* Return pressed key char */
	switch (cols | rows)
	{
		case 0x88:
			return KEYPAD_ROW0_COL0;
		case 0x48:
			return KEYPAD_ROW0_COL1;
		case 0x28:
			return KEYPAD_ROW0_COL2;
		case 0x18:
			return KEYPAD_ROW0_COL3;
		case 0x84:
			return KEYPAD_ROW1_COL0;
		case 0x44:
			return KEYPAD_ROW1_COL1;
		case 0x24:
			return KEYPAD_ROW1_COL2;
		case 0x14:
			return KEYPAD_ROW1_COL3;
		case 0x82:
			return KEYPAD_ROW2_COL0;
		case 0x42:
			return KEYPAD_ROW2_COL1;
		case 0x22:
			return KEYPAD_ROW2_COL2;
		case 0x12:
			return KEYPAD_ROW2_COL3;
		case 0x81:
			return KEYPAD_ROW3_COL0;
		case 0x41:
			return KEYPAD_ROW3_COL1;
		case 0x21:
			return KEYPAD_ROW3_COL2;
		case 0x11:
			return KEYPAD_ROW3_COL3;
		default:
			return KEYPAD_NO_PRESSED;
	}
}

/********************************* END OF FILE ********************************/
/******************************************************************************/
