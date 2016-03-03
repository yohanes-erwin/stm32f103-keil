#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "delay.h"
#include "lcd16x2.h"
#include "keypad4x4-scanning.h"

uint8_t key;

int main(void)
{
	DelayInit();
	lcd16x2_init(LCD16X2_DISPLAY_ON_CURSOR_OFF_BLINK_OFF);
	KeypadInit();
	
	while (1)
	{
		// Get key pressed
		key = KeypadGetKey();
		
		// Display pressed char to LCD
		if (key != KEYPAD_NO_PRESSED)
		{
			lcd16x2_gotoxy(0, 0);
			lcd16x2_putc(key);
			DelayMs(250);
		}
	}
}
