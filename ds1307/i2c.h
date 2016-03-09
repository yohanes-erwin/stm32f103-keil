/**
  ******************************************************************************
  * @file		i2c.h
  * @author	Yohanes Erwin Setiawan
  * @date		9 March 2016
  ******************************************************************************
  */

#ifndef __I2C_H
#define __I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/** Includes ---------------------------------------------------------------- */
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"

/** Defines ----------------------------------------------------------------- */
#define I2Cx_RCC				RCC_APB1Periph_I2C2
#define I2Cx						I2C2
#define I2C_GPIO_RCC		RCC_APB2Periph_GPIOB
#define I2C_GPIO				GPIOB
#define I2C_PIN_SDA			GPIO_Pin_11
#define I2C_PIN_SCL			GPIO_Pin_10

/** Public function prototypes ---------------------------------------------- */
void i2c_init(void);
void i2c_write_no_reg(uint8_t address, uint8_t data);
void i2c_write_with_reg(uint8_t address, uint8_t reg, uint8_t data);
void i2c_write_multi_no_reg(uint8_t address, uint8_t* data, uint8_t len);
void i2c_write_multi_with_reg(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len);
void i2c_read_no_reg(uint8_t address, uint8_t* data);
void i2c_read_with_reg(uint8_t address, uint8_t reg, uint8_t* data);
void i2c_read_multi_no_reg(uint8_t address, uint8_t len, uint8_t* data);
void i2c_read_multi_with_reg(uint8_t address, uint8_t reg, uint8_t len, uint8_t* data);

#ifdef __cplusplus
}
#endif

#endif

/********************************* END OF FILE ********************************/
/******************************************************************************/
