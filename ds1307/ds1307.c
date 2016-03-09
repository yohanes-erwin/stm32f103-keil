/**
  ******************************************************************************
  * @file		ds1307.c
  * @author	Yohanes Erwin Setiawan
  * @date		9 March 2016
  ******************************************************************************
  */
	
/** Includes ---------------------------------------------------------------- */	
#include "ds1307.h"

/** Private function prototypes --------------------------------------------- */
uint8_t bcd2bin(uint8_t bcd);
uint8_t bin2bcd(uint8_t bin);
uint8_t check_min_max(uint8_t val, uint8_t min, uint8_t max);

/** Public functions -------------------------------------------------------- */
/**
  ******************************************************************************
  *	@brief	Initialize I2C in master mode then connect to DS1307 chip
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void ds1307_init()
{
	// Initialize I2C
	i2c_init();
}

/**
  ******************************************************************************
  *	@brief	Get current seconds value from the register
  * @param	None
  * @retval	Current seconds value in decimal format
  ******************************************************************************
  */
uint8_t ds1307_get_seconds()
{
	uint8_t seconds;
	i2c_read_with_reg(DS1307_DEVICE_ADDRESS, DS1307_SECONDS, &seconds);
	
	return bcd2bin(seconds);
}

/**
  ******************************************************************************
  *	@brief	Set seconds value to the register
  * @param	Seconds value to be set
  * @retval	None
  ******************************************************************************
  */
void ds1307_set_seconds(uint8_t seconds)
{
	i2c_write_with_reg(DS1307_DEVICE_ADDRESS, DS1307_SECONDS, 
		bin2bcd(check_min_max(seconds, 0, 59)));
}

/**
  ******************************************************************************
  *	@brief	Get current minutes value from the register
  * @param	None
  * @retval	Current minutes value in decimal format
  ******************************************************************************
  */
uint8_t ds1307_get_minutes()
{
	uint8_t minutes;
	i2c_read_with_reg(DS1307_DEVICE_ADDRESS, DS1307_MINUTES, &minutes);
	
	return bcd2bin(minutes);
}

/**
  ******************************************************************************
  *	@brief	Set minutes value to the register
  * @param	Minutes value to be set
  * @retval	None
  ******************************************************************************
  */
void ds1307_set_minutes(uint8_t minutes)
{
	i2c_write_with_reg(DS1307_DEVICE_ADDRESS, DS1307_MINUTES, 
		bin2bcd(check_min_max(minutes, 0, 59)));
}

/**
  ******************************************************************************
  *	@brief	Get current hour mode (12 or 24)
  * @param	None
  * @retval	Current hour mode (DS1307_HOUR_12 or DS1307_HOUR_24)
  ******************************************************************************
  */
uint8_t ds1307_get_hour_mode()
{
	uint8_t hours;
	i2c_read_with_reg(DS1307_DEVICE_ADDRESS, DS1307_HOURS, &hours);
	
	// If hour mode bit is set, then current hour mode is 12, otherwise 24
	return ((hours & (1 << DS1307_HOUR_MODE)) == (1 << DS1307_HOUR_MODE)) ? 
		DS1307_HOUR_12 : DS1307_HOUR_24;
}

/**
  ******************************************************************************
  *	@brief	Get current am pm (AM or PM)
  * @param	None
  * @retval	Current am pm (DS1307_AM or DS1307_PM)
  ******************************************************************************
  */
uint8_t ds1307_get_am_pm()
{
	uint8_t hours;
	i2c_read_with_reg(DS1307_DEVICE_ADDRESS, DS1307_HOURS, &hours);
	
	// If AM PM bit is set, then current AM PM mode is PM, otherwise AM
	return ((hours & (1 << DS1307_AM_PM)) == (1 << DS1307_AM_PM)) ?
		DS1307_PM : DS1307_AM;
}

/**
  ******************************************************************************
  *	@brief	Get hours when hour mode is 12 
  * @param	None
  * @retval	Current hour value in decimal format
  ******************************************************************************
  */
uint8_t ds1307_get_hours_12()
{
	uint8_t hours;
	i2c_read_with_reg(DS1307_DEVICE_ADDRESS, DS1307_HOURS, &hours);
	
	// Mask hour register value from bit 7 to bit 5 to get hours value
	// in hour mode 12
	return bcd2bin(hours & 0x1F);
}

/**
  ******************************************************************************
  *	@brief	Set hours in hour mode 12
  * @param	Hours value
  * @param	AM or PM time
  * @retval	None
  ******************************************************************************
  */
void ds1307_set_hours_12(uint8_t hours_12, uint8_t am_pm)
{
	uint8_t hours = 0;
	if (am_pm == DS1307_AM)
	{
		// Hour mode is set, in order to use hour mode 12
		// AM/PM bit is clear, in order to set AM time
		hours = (1 << DS1307_HOUR_MODE) | 
			bin2bcd(check_min_max(hours_12, 1, 12));
	}
	else if (am_pm == DS1307_PM)
	{
		// Hour mode is set, in order to use hour mode 12
		// AM/PM bit is set, in order to set PM time
		hours = (1 << DS1307_HOUR_MODE) | (1 << DS1307_AM_PM) | 
			bin2bcd(check_min_max(hours_12, 1, 12));
	}
	i2c_write_with_reg(DS1307_DEVICE_ADDRESS, DS1307_HOURS, hours);
}

/**
  ******************************************************************************
  *	@brief	Get hours when hour mode is 24
  * @param	None
  * @retval	Current hour value in decimal format
  ******************************************************************************
  */
uint8_t ds1307_get_hours_24()
{
	uint8_t hours;
	i2c_read_with_reg(DS1307_DEVICE_ADDRESS, DS1307_HOURS, &hours);
	
	// Mask hour register value from bit 7 to bit 6 to get hours value
	// in hour mode 24
	return bcd2bin(hours & 0x3F);
}

/**
  ******************************************************************************
  *	@brief	Set hours in hour mode 24
  * @param	Hours value
  * @retval	None
  ******************************************************************************
  */
void ds1307_set_hours_24(uint8_t hours_24)
{
	i2c_write_with_reg(DS1307_DEVICE_ADDRESS, DS1307_HOURS, 
		bin2bcd(check_min_max(hours_24, 0, 23)));
}

/**
  ******************************************************************************
  *	@brief	Get current day of week value from the register
  * @param	None
  * @retval	Current day of week value in decimal format
  ******************************************************************************
  */
uint8_t ds1307_get_day()
{
	uint8_t day;
	i2c_read_with_reg(DS1307_DEVICE_ADDRESS, DS1307_DAY, &day);
		
	return bcd2bin(day);
}

/**
  ******************************************************************************
  *	@brief	Set day of week value to the register
  * @param	Day of week value to be set
  * @retval	None
  ******************************************************************************
  */
void ds1307_set_day(uint8_t day)
{
	i2c_write_with_reg(DS1307_DEVICE_ADDRESS, DS1307_DAY,
		bin2bcd(check_min_max(day, 1, 7)));
}

/**
  ******************************************************************************
  *	@brief	Get current date value from the register
  * @param	None
  * @retval	Current date value in decimal format
  ******************************************************************************
  */
uint8_t ds1307_get_date()
{
	uint8_t date;
	i2c_read_with_reg(DS1307_DEVICE_ADDRESS, DS1307_DATE, &date);
	
	return bcd2bin(date);
}

/**
  ******************************************************************************
  *	@brief	Set date value to the register
  * @param	Date value to be set
  * @retval	None
  ******************************************************************************
  */
void ds1307_set_date(uint8_t date)
{
	i2c_write_with_reg(DS1307_DEVICE_ADDRESS, DS1307_DATE,
		bin2bcd(check_min_max(date, 1, 31)));
}

/**
  ******************************************************************************
  *	@brief	Get current month value from the register
  * @param	None
  * @retval	Current month value in decimal format
  ******************************************************************************
  */
uint8_t ds1307_get_month()
{
	uint8_t month;
	i2c_read_with_reg(DS1307_DEVICE_ADDRESS, DS1307_MONTH, &month);
	
	return bcd2bin(month);
}

/**
  ******************************************************************************
  *	@brief	Set month value to the register
  * @param	Month value to be set
  * @retval	None
  ******************************************************************************
  */
void ds1307_set_month(uint8_t month)
{
	i2c_write_with_reg(DS1307_DEVICE_ADDRESS, DS1307_MONTH,
		bin2bcd(check_min_max(month, 1, 12)));
}

/**
  ******************************************************************************
  *	@brief	Get current year value from the register
  * @param	None
  * @retval	Current year value in decimal format
  ******************************************************************************
  */
uint8_t ds1307_get_year()
{
	uint8_t year;
	i2c_read_with_reg(DS1307_DEVICE_ADDRESS, DS1307_YEAR, &year);
	
	return bcd2bin(year);
}

/**
  ******************************************************************************
  *	@brief	Set year value to the register
  * @param	Year value to be set
  * @retval	None
  ******************************************************************************
  */
void ds1307_set_year(uint8_t year)
{
	i2c_write_with_reg(DS1307_DEVICE_ADDRESS, DS1307_YEAR,
		bin2bcd(check_min_max(year, 0, 99)));
}

/**
  ******************************************************************************
  *	@brief	Get hours, minutes, seconds, and AM/PM value from the register in 12 
  *					hour mode
  * @param	Pointer to variable for store hours value
  * @param	Pointer to variable for store minutes value 
  * @param	Pointer to variable for store seconds value 
  * @param	Pointer to variable for store AM/PM value  
  * @retval	None
  ******************************************************************************
  */
void ds1307_get_time_12(uint8_t* hours, uint8_t* minutes, uint8_t* seconds, uint8_t* am_pm)
{
	*hours = ds1307_get_hours_12();
	*minutes = ds1307_get_minutes();
	*seconds = ds1307_get_seconds();
	*am_pm = ds1307_get_am_pm();
}

/**
  ******************************************************************************
  *	@brief	Set hours, minutes, seconds, and AM/PM value to the register in 12 
  *					hour mode
  * @param	Hours value to be set
  * @param	Minutes value to be set
  * @param	Seconds value to be set
  * @param	AM/PM value to be set
  * @retval	None
  ******************************************************************************
  */
void ds1307_set_time_12(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t am_pm)
{
	ds1307_set_hours_12(hours, am_pm);
	ds1307_set_minutes(minutes);
	ds1307_set_seconds(seconds);
}

/**
  ******************************************************************************
  *	@brief	Get hours, minutes, and seconds value from the register in 24 
  *					hour mode
  * @param	Pointer to variable for store hours value
  * @param	Pointer to variable for store minutes value
  * @param	Pointer to variable for store seconds value
  * @retval	None
  ******************************************************************************
  */
void ds1307_get_time_24(uint8_t* hours, uint8_t* minutes, uint8_t* seconds)
{
	*hours = ds1307_get_hours_24();
	*minutes = ds1307_get_minutes();
	*seconds = ds1307_get_seconds();
}

/**
  ******************************************************************************
  *	@brief	Set hours, minutes, and seconds value to the register in 24 
  *					hour mode
  * @param	Hours value to be set
  * @param	Minutes value to be set
  * @param	Seconds value to be set
  * @retval	None
  ******************************************************************************
  */
void ds1307_set_time_24(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	ds1307_set_hours_24(hours);
	ds1307_set_minutes(minutes);
	ds1307_set_seconds(seconds);
}

/**
  ******************************************************************************
  *	@brief	Get day, date, month, and year value from the register
  * @param	Pointer to variable for store day value
  * @param	Pointer to variable for store date value
  * @param	Pointer to variable for store month value
  * @param	Pointer to variable for store year value
  * @retval	None
  ******************************************************************************
  */
void ds1307_get_calendar_date(uint8_t* day, uint8_t* date, uint8_t* month, uint8_t* year)
{
	*day = ds1307_get_day();
	*date = ds1307_get_date();
	*month = ds1307_get_month();
	*year = ds1307_get_year();
}

/**
  ******************************************************************************
  *	@brief	Set day, date, month, and year value to the register
  * @param	Day value to be set
  * @param	Date value to be set
  * @param	Month value to be set
  * @param	Year value to be set
  * @retval	None
  ******************************************************************************
  */
void ds1307_set_calendar_date(uint8_t day, uint8_t date, uint8_t month, uint8_t year)
{
	ds1307_set_day(day);
	ds1307_set_date(date);
	ds1307_set_month(month);
	ds1307_set_year(year);
}

/** Private functions ------------------------------------------------------- */
/**
  ******************************************************************************
  *	@brief	Convert from BCD format to BIN format
  * @param	BCD value to be converted
  * @retval	BIN value from given BCD
  ******************************************************************************
  */
uint8_t bcd2bin(uint8_t bcd)
{
	uint8_t bin = (bcd >> 4) * 10;
	bin += bcd & 0x0F;
	
	return bin;
}

/**
  ******************************************************************************
  *	@brief	Convert from BIN format to BCD format
  * @param	BIN value to be converted
  * @retval	BCD value from given BIN
  ******************************************************************************
  */
uint8_t bin2bcd(uint8_t bin)
{
	uint8_t high = bin / 10;
	uint8_t low = bin - (high *10);
	
	return (high << 4) | low;
}

/**
  ******************************************************************************
  *	@brief	Check min and max from given value
  * @param	The value to be checked
  * @param	Allowed minimum value
  * @param	Allowed maximum value
  * @retval	Value between min and max or equal min or max
  ******************************************************************************
  */
uint8_t check_min_max(uint8_t val, uint8_t min, uint8_t max)
{
	if (val < min)
		return min;
	else if (val > max)
		return max;
	
	return val;
}

/********************************* END OF FILE ********************************/
/******************************************************************************/
