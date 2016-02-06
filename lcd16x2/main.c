#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "delay.h"
#include "lcd16x2.h"

// Custom char data (battery symbol)
uint8_t custom_char[] = { 0x0E, 0x1B, 0x11, 0x11, 0x11, 0x11, 0x1F, 0x1F };

int main(void)
{
	// Delay initialization
	DelayInit();
	
	// LCD initialization
	lcd16x2_init(LCD16X2_DISPLAY_ON_CURSOR_OFF_BLINK_OFF);
	
	// Create custom char
	lcd16x2_create_custom_char(0, custom_char);
	
	while (1)
	{
		// Display custom char
		lcd16x2_put_custom_char(0, 0, 0);
		lcd16x2_puts(" Battery Low");
		DelayMs(500);
		// Clear display
		lcd16x2_clrscr();
		DelayMs(500);
	}
}
