#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
#include "delay.h"
#include "lcd16x2.h"
#include "i2c.h"
#include <stdio.h>

#define HMC5833L_DEVICE_ADDRESS				0x1E
#define HMC5833L_CFG_A_REGISTER				0x00
#define HMC5833L_CFG_B_REGISTER				0x01
#define HMC5833L_MODE_REGISTER				0x02
#define HMC5833L_DO_REGISTER_START		0x03

void HMC5883_Init(void);
void HMC5883_GetHeadings(void);

int16_t rawX, rawY, rawZ;
char buf[8];

int main(void)
{
	DelayInit();
	lcd16x2_init(LCD16X2_DISPLAY_ON_CURSOR_OFF_BLINK_OFF);
	
	// Initialize HMC5883L
	HMC5883_Init();
	
	while (1)
	{
		HMC5883_GetHeadings();
		
		lcd16x2_clrscr();
		lcd16x2_puts("X,Y,Z =\n");
		sprintf(buf, "%d,", rawX);
		lcd16x2_puts(buf);
		sprintf(buf, "%d,", rawY);
		lcd16x2_puts(buf);
		sprintf(buf, "%d", rawZ);
		lcd16x2_puts(buf);
		
		DelayMs(250);
	}
}

void HMC5883_Init()
{
	// Initialize I2C
	i2c_init();
	
	// Set HMC5883L to default configuration (from datasheet) 
	i2c_write_with_reg(HMC5833L_DEVICE_ADDRESS, HMC5833L_CFG_A_REGISTER, 0x70);
	i2c_write_with_reg(HMC5833L_DEVICE_ADDRESS, HMC5833L_CFG_B_REGISTER, 0xA0);
	// Set HMC5883L to continuous measurement mode
	i2c_write_with_reg(HMC5833L_DEVICE_ADDRESS, HMC5833L_MODE_REGISTER, 0x00);
}

void HMC5883_GetHeadings()
{
	// Raw value from data output register 
	// (X MSB, X LSB, Z MSB, Z LSB, Y MSB, Y LSB)
	uint8_t rawValue[6];
	i2c_read_multi_with_reg(HMC5833L_DEVICE_ADDRESS, 0x03, 6, rawValue);
	
	// Store MSB and LSB of raw X value
	rawX = rawValue[0] << 8;
	rawX |= rawValue[1];
	
	// Store MSB and LSB of raw Z value
	rawZ = rawValue[2] << 8;
	rawZ |= rawValue[3];
	
	// Store MSB and LSB of raw Y	value
	rawY = rawValue[4] << 8;
	rawY |= rawValue[5];
}
