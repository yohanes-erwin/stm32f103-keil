/**
  ******************************************************************************
  * @file		ds1307.h
  * @author	Yohanes Erwin Setiawan
  * @date		9 March 2016
  ******************************************************************************
  */
	
#ifndef __DS1307_H
#define __DS1307_H

#ifdef __cplusplus
extern "C" {
#endif

/** Includes ---------------------------------------------------------------- */	
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "i2c.h"	

/** Defines ----------------------------------------------------------------- */
// Slave address for DS1307 chip (7-bit format)
#define DS1307_DEVICE_ADDRESS		0x68

// Register location
#define DS1307_SECONDS			0x00
#define DS1307_MINUTES			0x01
#define DS1307_HOURS				0x02
#define DS1307_DAY					0x03
#define DS1307_DATE					0x04
#define DS1307_MONTH				0x05
#define DS1307_YEAR					0x06

// Bits in hours register
#define DS1307_HOUR_MODE		6
#define DS1307_AM_PM				5

// DS1307 hour mode
#define DS1307_HOUR_12			1
#define DS1307_HOUR_24			0
#define DS1307_AM						0
#define DS1307_PM						1

// DS1307 day of week
#define DS1307_SUNDAY				1
#define DS1307_MONDAY				2
#define DS1307_TUESDAY			3
#define DS1307_WEDNESDAY		4
#define DS1307_THRUSDAY			5
#define DS1307_FRIDAY				6
#define DS1307_SATURDAY			7

/** Public function prototypes ---------------------------------------------- */
void ds1307_init(void);
uint8_t ds1307_get_seconds(void); 
void ds1307_set_seconds(uint8_t seconds);
uint8_t ds1307_get_minutes(void); 
void ds1307_set_minutes(uint8_t minutes);
uint8_t ds1307_get_hour_mode(void);
uint8_t ds1307_get_am_pm(void);
uint8_t ds1307_get_hours_12(void);
void ds1307_set_hours_12(uint8_t hours_12, uint8_t am_pm);
uint8_t ds1307_get_hours_24(void);
void ds1307_set_hours_24(uint8_t hours_24);
uint8_t ds1307_get_day(void); 
void ds1307_set_day(uint8_t day);
uint8_t ds1307_get_date(void); 
void ds1307_set_date(uint8_t date);
uint8_t ds1307_get_month(void); 
void ds1307_set_month(uint8_t month);
uint8_t ds1307_get_year(void);  
void ds1307_set_year(uint8_t year);
void ds1307_get_time_12(uint8_t* hours, uint8_t* minutes, uint8_t* seconds, uint8_t* am_pm);
void ds1307_set_time_12(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t am_pm);
void ds1307_get_time_24(uint8_t* hours, uint8_t* minutes, uint8_t* seconds);
void ds1307_set_time_24(uint8_t hours, uint8_t minutes, uint8_t seconds);
void ds1307_get_calendar_date(uint8_t* day, uint8_t* date, uint8_t* month, uint8_t* year);
void ds1307_set_calendar_date(uint8_t day, uint8_t date, uint8_t month, uint8_t year);

#ifdef __cplusplus
}
#endif

#endif

/********************************* END OF FILE ********************************/
/******************************************************************************/
