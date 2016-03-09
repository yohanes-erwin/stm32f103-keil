#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
#include "delay.h"
#include "lcd16x2.h"
#include "ds1307.h"
#include "i2c.h"
#include <stdio.h>

uint8_t D, d, M, y, h, m, s;
char buf[4];

int main(void)
{
	DelayInit();
	lcd16x2_init(LCD16X2_DISPLAY_ON_CURSOR_OFF_BLINK_OFF);
	
	// Initialize DS1307
	ds1307_init();
	
	// Set initial date and time 
	ds1307_set_calendar_date(DS1307_WEDNESDAY, 9, 3, 16);
	ds1307_set_time_24(10, 10, 30);
	
	while (1)
	{
		// Get date and time
		ds1307_get_calendar_date(&D, &d, &M, &y);
		ds1307_get_time_24(&h, &m, &s);
		
		// Display date and time to LCD
		lcd16x2_clrscr();
		
		lcd16x2_gotoxy(3, 0);
		sprintf(buf, (d <= 9) ? "0%d" : "%d", d);
		lcd16x2_puts(buf);
		lcd16x2_puts("/");
		sprintf(buf, (M <= 9) ? "0%d" : "%d", M);
		lcd16x2_puts(buf);
		lcd16x2_puts("/");
		sprintf(buf, "20%d", y);
		lcd16x2_puts(buf);
		
		lcd16x2_gotoxy(4, 1);
		sprintf(buf, (h <= 9) ? "0%d" : "%d", h);
		lcd16x2_puts(buf);
		lcd16x2_puts(":");
		sprintf(buf, (m <= 9) ? "0%d" : "%d", m);
		lcd16x2_puts(buf);
		lcd16x2_puts(":");
		sprintf(buf, (s <= 9) ? "0%d" : "%d", s);
		lcd16x2_puts(buf);
		
		DelayMs(1000);
	}
}
