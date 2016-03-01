/**
  ******************************************************************************
  * @file		lcd16x2.c
  * @author	Yohanes Erwin Setiawan
  * @date		6 February 2016
	* @note		Re-write form Peter Fleury AVR LCD library
  ******************************************************************************
  */

/** Includes ---------------------------------------------------------------- */
#include "lcd16x2.h"

/** Private function prototypes --------------------------------------------- */
static void lcd16x2_toggle_e(void);
static void lcd16x2_write(uint8_t data, uint8_t rs);
static uint8_t lcd16x2_read(uint8_t rs);
static uint8_t lcd16x2_wait_busy(void);
static void lcd16x2_new_line(uint8_t pos);

static uint8_t display_cursor_on_off_control;
GPIO_InitTypeDef GPIO_InitStruct;

/** Public functions -------------------------------------------------------- */
/**
  ******************************************************************************
  * @brief	Initialize the LCD 16x2 with 4-bit I/O mode.
  * @param	Display, cursor underline, and cursor blink settings. See
  * 				LCD display and cursor attributes define in lcd16x2.h file.
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_init(uint8_t disp_attr)
{
	// Delay initialization
	DelayInit();
	
	// GPIO clock for control and data lines
	RCC_APB2PeriphClockCmd(LCD16X2_RCC_GPIO_CONTROL, ENABLE);
	RCC_APB2PeriphClockCmd(LCD16X2_RCC_GPIO_DATA, ENABLE);
	
	// Configure I/O for control lines as output
	GPIO_InitStruct.GPIO_Pin = LCD16X2_PIN_RS | LCD16X2_PIN_RW | 
		LCD16X2_PIN_EN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(LCD16X2_GPIO_CONTROL, &GPIO_InitStruct);
	
	// Configure I/O for data lines as output
	GPIO_InitStruct.GPIO_Pin = LCD16X2_PIN_D4 | LCD16X2_PIN_D5 | 
		LCD16X2_PIN_D6 | LCD16X2_PIN_D7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(LCD16X2_GPIO_DATA, &GPIO_InitStruct);

	// Delay power on 
	DelayUs(LCD16X2_DELAY_POWER_ON);
	
	// Initialize 8-bit mode first
	LCD16X2_GPIO_D5->BSRR = LCD16X2_PIN_D5;	// Function set
	LCD16X2_GPIO_D4->BSRR = LCD16X2_PIN_D4;	// 8-bit mode
	lcd16x2_toggle_e();
	// Delay, busy flag can't be checked here
	DelayUs(LCD16X2_DELAY_INIT);
	
	// Repeat last command
	lcd16x2_toggle_e();
	// Delay, busy flag can't be checked here
	DelayUs(LCD16X2_DELAY_INIT_REP);
	
	// Repeat last command for third time
	lcd16x2_toggle_e();
	// Delay, busy flag can't be checked here
	DelayUs(LCD16X2_DELAY_INIT_REP);
	
	// Initialize 4-bit mode
	LCD16X2_GPIO_D5->BSRR = LCD16X2_PIN_D5;	// Function set
	LCD16X2_GPIO_D4->BRR = LCD16X2_PIN_D4;	// 4-bit mode
	lcd16x2_toggle_e();
	DelayUs(LCD16X2_DELAY_INIT_4BIT);
	
	/* From now the LCD only accepts 4 bit I/O */
	
	// 4-bit interface, 2 lines, 5x7 dot format font
	lcd16x2_write_command(LCD16X2_FUNCTION_SET | LCD16X2_4BIT_INTERFACE | 
		LCD16X2_2LINE_MODE | LCD16X2_5X7DOT_FORMAT);
	// Display off
	lcd16x2_write_command(LCD16X2_DISPLAY_CURSOR_ON_OFF | LCD16X2_DISPLAY_OFF);
	// Clear screen
	lcd16x2_clrscr();
	// Entry mode
	lcd16x2_entry_inc();
	// Display cursor on off
	display_cursor_on_off_control = disp_attr;
	lcd16x2_write_command(LCD16X2_DISPLAY_CURSOR_ON_OFF | 
		display_cursor_on_off_control);
}

/**
  ******************************************************************************
  * @brief	Write a command to the LCD.
  * @param	The LCD instructions set.
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_write_command(uint8_t cmd)
{
	lcd16x2_wait_busy();
	lcd16x2_write(cmd, 0);
}

/**
  ******************************************************************************
  * @brief	Write a data byte to the LCD.
  * @param	Data which want to written to the LCD.
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_write_data(uint8_t data)
{
	lcd16x2_wait_busy();
	lcd16x2_write(data, 1);
}

/**
  ******************************************************************************
  * @brief	Clear the LCD display and return cursor to home position.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_clrscr()
{
	lcd16x2_write_command(LCD16X2_CLEAR_DISPLAY);
}

/**
  ******************************************************************************
  * @brief	Return cursor to home position.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_home()
{
	lcd16x2_write_command(LCD16X2_CURSOR_HOME);	
}

/**
  ******************************************************************************
  * @brief	Set LCD cursor to specific position.
  * @param	LCD column (x)
  * @param	LCD row (y)
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_gotoxy(uint8_t x, uint8_t y)
{
#if LCD16X2_LINES == 1 
	lcd16X2_write_command(LCD16X2_SET_DDRAM_ADDRESS |
		(LCD16X2_START_LINE_1 + x));
#elif LCD16X2_LINES == 2
	if (y == 0)
		lcd16x2_write_command(LCD16X2_SET_DDRAM_ADDRESS | 
			(LCD16X2_START_LINE_1 + x));
	else
		lcd16x2_write_command(LCD16X2_SET_DDRAM_ADDRESS | 
			(LCD16X2_START_LINE_2 + x));
#endif
}

/**
  ******************************************************************************
  * @brief	Get LCD cursor/ DDRAM address.
  * @param	None
  * @retval	LCD cursor/ DDRAM address.
  ******************************************************************************
  */
uint8_t lcd16x2_getxy()
{
	return lcd16x2_wait_busy();
}

/**
  ******************************************************************************
  * @brief	Set LCD entry mode: increment cursor.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_entry_inc()
{
	lcd16x2_write_command(LCD16X2_CHARACTER_ENTRY_MODE | LCD16X2_INCREMENT |
		LCD16X2_DISPLAY_SHIFT_OFF);
}

/**
  ******************************************************************************
  * @brief	Set LCD entry mode: decrement cursor.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_entry_dec()
{
	lcd16x2_write_command(LCD16X2_CHARACTER_ENTRY_MODE | LCD16X2_DECREMENT |
		LCD16X2_DISPLAY_SHIFT_OFF);
}

/**
  ******************************************************************************
  * @brief	Set LCD entry mode: increment cursor and shift character to left.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_entry_inc_shift()
{
	lcd16x2_write_command(LCD16X2_CHARACTER_ENTRY_MODE | LCD16X2_INCREMENT |
		LCD16X2_DISPLAY_SHIFT_ON);
}

/**
  ******************************************************************************
  * @brief	Set LCD entry mode: decrement cursor and shift character to right.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_entry_dec_shift()
{
	lcd16x2_write_command(LCD16X2_CHARACTER_ENTRY_MODE | LCD16X2_DECREMENT |
		LCD16X2_DISPLAY_SHIFT_ON);
}

/**
  ******************************************************************************
  * @brief	Turn on display (can see character(s) on display).
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_display_on()
{
	display_cursor_on_off_control |= LCD16X2_DISPLAY_ON;
	lcd16x2_write_command(LCD16X2_DISPLAY_CURSOR_ON_OFF | 
		display_cursor_on_off_control);	
}

/**
  ******************************************************************************
  * @brief	Turn off display (blank/ can't see character(s) on display).
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_display_off()
{
	display_cursor_on_off_control &= ~LCD16X2_DISPLAY_ON;
	lcd16x2_write_command(LCD16X2_DISPLAY_CURSOR_ON_OFF | 
		display_cursor_on_off_control);	
}

/**
  ******************************************************************************
  * @brief	Turn on underline cursor.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_cursor_on()
{
	display_cursor_on_off_control |= LCD16X2_CURSOR_UNDERLINE_ON; 	
	lcd16x2_write_command(LCD16X2_DISPLAY_CURSOR_ON_OFF | 
		display_cursor_on_off_control);
}

/**
  ******************************************************************************
  * @brief	Turn off underline cursor.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_cursor_off()
{
	display_cursor_on_off_control &= ~LCD16X2_CURSOR_UNDERLINE_ON;
	lcd16x2_write_command(LCD16X2_DISPLAY_CURSOR_ON_OFF | 
		display_cursor_on_off_control);
}

/**
  ******************************************************************************
  * @brief	Turn on blinking cursor.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_blink_on()
{
	display_cursor_on_off_control |= LCD16X2_CURSOR_BLINK_ON;
	lcd16x2_write_command(LCD16X2_DISPLAY_CURSOR_ON_OFF | 
		display_cursor_on_off_control);
}

/**
  ******************************************************************************
  * @brief	Turn off blinking cursor.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_blink_off()
{
	display_cursor_on_off_control &= ~LCD16X2_CURSOR_BLINK_ON;
	lcd16x2_write_command(LCD16X2_DISPLAY_CURSOR_ON_OFF | 
		display_cursor_on_off_control);
}

/**
  ******************************************************************************
  * @brief	Shift the LCD display to the left.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_display_shift_left()
{
	lcd16x2_write_command(LCD16X2_DISPLAY_CURSOR_SHIFT | 
		LCD16X2_DISPLAY_SHIFT | LCD16X2_LEFT_SHIFT);
}

/**
  ******************************************************************************
  * @brief	Shift the LCD display to the right.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_display_shift_right()
{
	lcd16x2_write_command(LCD16X2_DISPLAY_CURSOR_SHIFT |
		LCD16X2_DISPLAY_SHIFT | LCD16X2_RIGHT_SHIFT);
}

/**
  ******************************************************************************
  * @brief	Shift the LCD cursor to the left (DDRAM address incremented).
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_cursor_shift_left()
{
	lcd16x2_write_command(LCD16X2_DISPLAY_CURSOR_SHIFT |
		LCD16X2_DISPLAY_CURSOR_SHIFT | LCD16X2_LEFT_SHIFT);
}

/**
  ******************************************************************************
  * @brief	Shift the LCD cursor to the right (DDRAM address decremented).
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_cursor_shift_right()
{
	lcd16x2_write_command(LCD16X2_DISPLAY_CURSOR_SHIFT |
		LCD16X2_DISPLAY_CURSOR_SHIFT | LCD16X2_RIGHT_SHIFT);
}

/**
  ******************************************************************************
  * @brief	Put a character on the LCD display.
  * @param	Character that want to be displayed.
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_putc(const char c)
{
	uint8_t pos = lcd16x2_getxy();

	if (c == '\n')
	{
		lcd16x2_new_line(pos);
	}
	else
	{
#if LCD16X2_LINES == 1
		if (pos == (LCD16X2_START_LINE_1 + LCD16X2_DISP_LENGTH))
			lcd16x2_write(LCD16X2_SET_DDRAM_ADDRESS |
				LCD16X2_START_LINE_1, 0);
#elif LCD16X2_LINES == 2
		if (pos == (LCD16X2_START_LINE_1 + LCD16X2_DISP_LENGTH))
			lcd16x2_write(LCD16X2_SET_DDRAM_ADDRESS |
				LCD16X2_START_LINE_2, 0);
		else if (pos == (LCD16X2_START_LINE_2 + LCD16X2_DISP_LENGTH))
			lcd16x2_write(LCD16X2_SET_DDRAM_ADDRESS |
				LCD16X2_START_LINE_1, 0);
#endif
		
		lcd16x2_write_data(c);
	}
}

/**
  ******************************************************************************
  * @brief	Put string on the LCD display.
  * @param	String that want to be displayed.
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_puts(const char* s)
{
	while (*s) {
		lcd16x2_putc(*s++);
	}
}

/**
  ******************************************************************************
  * @brief	Create a custom character on CGRAM location.
  * @param	CGRAM location (0-7).
  * @param	Custom character pattern (8 bytes).
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_create_custom_char(uint8_t location, uint8_t* data_bytes)
{
	int i;
	
	// We only have 8 locations 0-7 for custom chars
	location &= 0x07; 
	
	// Set CGRAM address
	lcd16x2_write_command(LCD16X2_SET_CGRAM_ADDRESS | (location << 3));
	
	// Write 8 bytes custom char pattern
	for (i = 0; i < 8; i++) 
	{
		lcd16x2_write_data(data_bytes[i]);
	}
}

/**
  ******************************************************************************
  * @brief	Put a custom character on specific LCD display location.
  * @param	LCD column
  * @param	LCD row
  * @param	Custom character location on CGRAM (0-7).
  * @retval	None
  ******************************************************************************
  */
void lcd16x2_put_custom_char(uint8_t x, uint8_t y, uint8_t location)
{
	lcd16x2_gotoxy(x, y);
	lcd16x2_write_data(location);
}

/** Private functions ------------------------------------------------------- */
/**
  ******************************************************************************
  * @brief	Give enable pulse to LCD EN pin.
  * @param	None
  * @retval	None
  ******************************************************************************
  */
static void lcd16x2_toggle_e()
{
	// EN pin = HIGH
	LCD16X2_GPIO_EN->BSRR = LCD16X2_PIN_EN;
	// Pulse length in us
	DelayUs(LCD16X2_DELAY_ENABLE_PULSE);
	// EN pin = LOW
	LCD16X2_GPIO_EN->BRR = LCD16X2_PIN_EN;
}

/**
  ******************************************************************************
  * @brief	Write instruction or data to LCD.
  * @param	Instruction/ data that want to sent to LCD.
  * @param	Instruction or data register select. If write instruction, then 
  *					RS = 0. Otherwise, RS = 1.
  * @retval	None
  ******************************************************************************
  */
static void lcd16x2_write(uint8_t data, uint8_t rs)
{
	// Write mode (RW = 0)
	LCD16X2_GPIO_RS->BRR = LCD16X2_PIN_RW;
	
	if (rs)
		// Write data (RS = 1)
		LCD16X2_GPIO_RS->BSRR = LCD16X2_PIN_RS;
	else		
		// Write instruction (RS = 0)
		LCD16X2_GPIO_RS->BRR = LCD16X2_PIN_RS;
	
	// Configure all data pins as output
	GPIO_InitStruct.GPIO_Pin = LCD16X2_PIN_D4 | LCD16X2_PIN_D5 | 
		LCD16X2_PIN_D6 | LCD16X2_PIN_D7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(LCD16X2_GPIO_DATA, &GPIO_InitStruct);
	
	// Output high nibble first
	LCD16X2_GPIO_D7->BRR = LCD16X2_PIN_D7;
	LCD16X2_GPIO_D6->BRR = LCD16X2_PIN_D6;
	LCD16X2_GPIO_D5->BRR = LCD16X2_PIN_D5;
	LCD16X2_GPIO_D4->BRR = LCD16X2_PIN_D4;
	if (data & 0x80) LCD16X2_GPIO_D7->BSRR = LCD16X2_PIN_D7;
	if (data & 0x40) LCD16X2_GPIO_D7->BSRR = LCD16X2_PIN_D6;
	if (data & 0x20) LCD16X2_GPIO_D7->BSRR = LCD16X2_PIN_D5;
	if (data & 0x10) LCD16X2_GPIO_D7->BSRR = LCD16X2_PIN_D4;
	lcd16x2_toggle_e();
	
	// Output low nibble
	LCD16X2_GPIO_D7->BRR = LCD16X2_PIN_D7;
	LCD16X2_GPIO_D6->BRR = LCD16X2_PIN_D6;
	LCD16X2_GPIO_D5->BRR = LCD16X2_PIN_D5;
	LCD16X2_GPIO_D4->BRR = LCD16X2_PIN_D4;
	if (data & 0x08) LCD16X2_GPIO_D7->BSRR = LCD16X2_PIN_D7;
	if (data & 0x04) LCD16X2_GPIO_D7->BSRR = LCD16X2_PIN_D6;
	if (data & 0x02) LCD16X2_GPIO_D7->BSRR = LCD16X2_PIN_D5;
	if (data & 0x01) LCD16X2_GPIO_D7->BSRR = LCD16X2_PIN_D4;
	lcd16x2_toggle_e();
	
	// All data pins high (inactive)
	LCD16X2_GPIO_D7->BSRR = LCD16X2_PIN_D7;
	LCD16X2_GPIO_D7->BSRR = LCD16X2_PIN_D6;
	LCD16X2_GPIO_D7->BSRR = LCD16X2_PIN_D5;
	LCD16X2_GPIO_D7->BSRR = LCD16X2_PIN_D4;
}

/**
  ******************************************************************************
  * @brief	Read DDRAM address + busy flag or data from LCD.
  * @param	DDRAM address + busy flag or data register select. 
  *					If read DDRAM address + busy flag, then RS = 0. Otherwise, RS = 1.
  * @retval	DDRAM address + busy flag or data value.
  ******************************************************************************
  */
static uint8_t lcd16x2_read(uint8_t rs)
{
	uint8_t data = 0;
	
	// Read mode (RW = 1)
	LCD16X2_GPIO_RS->BSRR = LCD16X2_PIN_RW;
	
	if (rs)
		// Read data (RS = 1)
		LCD16X2_GPIO_RS->BSRR = LCD16X2_PIN_RS;
	else
		// Read busy flag and DDRAM address (RS = 0)
		LCD16X2_GPIO_RS->BRR = LCD16X2_PIN_RS;
		
	// Configure all data pins as input
	GPIO_InitStruct.GPIO_Pin = LCD16X2_PIN_D4 | LCD16X2_PIN_D5 | 
		LCD16X2_PIN_D6 | LCD16X2_PIN_D7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(LCD16X2_GPIO_DATA, &GPIO_InitStruct);
	
	// EN pin = HIGH
	LCD16X2_GPIO_EN->BSRR = LCD16X2_PIN_EN;
	// Pulse length in us
	DelayUs(LCD16X2_DELAY_ENABLE_PULSE);
	/* Read high nibble first */
	if (LCD16X2_GPIO_D4->IDR & LCD16X2_PIN_D4) data |= 0x10;	
	if (LCD16X2_GPIO_D5->IDR & LCD16X2_PIN_D5) data |= 0x20;
	if (LCD16X2_GPIO_D6->IDR & LCD16X2_PIN_D6) data |= 0x40;
	if (LCD16X2_GPIO_D7->IDR & LCD16X2_PIN_D7) data |= 0x80;
	// EN pin = LOW
	LCD16X2_GPIO_EN->BRR = LCD16X2_PIN_EN;
	
	// EN pin LOW delay
	DelayUs(LCD16X2_DELAY_ENABLE_PULSE);
	
	// EN pin = HIGH
	LCD16X2_GPIO_EN->BSRR = LCD16X2_PIN_EN;
	// Pulse length in us
	DelayUs(LCD16X2_DELAY_ENABLE_PULSE);
	/* Read low nibble */
	if (LCD16X2_GPIO_D4->IDR & LCD16X2_PIN_D4) data |= 0x01;
	if (LCD16X2_GPIO_D5->IDR & LCD16X2_PIN_D5) data |= 0x02;
	if (LCD16X2_GPIO_D6->IDR & LCD16X2_PIN_D6) data |= 0x04;
	if (LCD16X2_GPIO_D7->IDR & LCD16X2_PIN_D7) data |= 0x08;
	// EN pin = LOW
	LCD16X2_GPIO_EN->BRR = LCD16X2_PIN_EN;
	
	return data;
}

/**
  ******************************************************************************
  * @brief	Wait for LCD until finish it's job.
  * @param	None
  * @retval	DDRAM address + busy flag value.
  ******************************************************************************
  */
static uint8_t lcd16x2_wait_busy()
{
	// Wait until busy flag is cleared
	while (lcd16x2_read(0) & (LCD16X2_BUSY_FLAG));
	
	// Delay needed for address counter is updated after busy flag is cleared
	DelayUs(LCD16X2_DELAY_BUSY_FLAG);
	
	// Read and return address counter
	return lcd16x2_read(0);
}

/**
  ******************************************************************************
  * @brief	Give new line character 
  * @param	Current cursor/ DDRAM address position.
  * @retval	None
  ******************************************************************************
  */
static void lcd16x2_new_line(uint8_t pos)
{
	uint8_t address_counter;
	
#if LCD16X2_LINES == 1
	address_counter = 0;
#elif LCD16X2_LINES == 2
	if (pos < LCD16X2_START_LINE_2)
		address_counter = LCD16X2_START_LINE_2;
	else 
		address_counter = LCD16X2_START_LINE_1;
#endif

	lcd16x2_write_command(LCD16X2_SET_DDRAM_ADDRESS | address_counter);
}

/********************************* END OF FILE ********************************/
/******************************************************************************/
