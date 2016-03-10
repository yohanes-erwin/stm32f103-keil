/**
  ******************************************************************************
  * @file		i2c.c
  * @author	Yohanes Erwin Setiawan
  * @date		9 March 2016
  ******************************************************************************
  */

/** Includes ---------------------------------------------------------------- */	
#include "i2c.h"

/** Private function prototypes --------------------------------------------- */
void i2c_start(void);
void i2c_stop(void);
void i2c_address_direction(uint8_t address, uint8_t direction);
void i2c_transmit(uint8_t byte);
uint8_t i2c_receive_ack(void);
uint8_t i2c_receive_nack(void);

/** Public functions -------------------------------------------------------- */
/**
  ******************************************************************************
  *	@brief	Initialize I2C in master mode
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void i2c_init()
{
	// Initialization struct
	I2C_InitTypeDef I2C_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Step 1: Initialize I2C
	RCC_APB1PeriphClockCmd(I2Cx_RCC, ENABLE);
	I2C_InitStruct.I2C_ClockSpeed = 100000;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2Cx, &I2C_InitStruct);
	I2C_Cmd(I2Cx, ENABLE);
	
	// Step 2: Initialize GPIO as open drain alternate function
	RCC_APB2PeriphClockCmd(I2C_GPIO_RCC, ENABLE);
	GPIO_InitStruct.GPIO_Pin = I2C_PIN_SCL | I2C_PIN_SDA;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(I2C_GPIO, &GPIO_InitStruct);
}

/**
  ******************************************************************************
  *	@brief	Write byte to slave without specify register address
  * @param	Slave device address (7-bit right aligned)
  * @param	Data byte
  * @retval	None
  ******************************************************************************
  */
void i2c_write_no_reg(uint8_t address, uint8_t data)
{
	i2c_start();
	i2c_address_direction(address << 1, I2C_Direction_Transmitter);
	i2c_transmit(data);
	i2c_stop();
}

/**
  ******************************************************************************
  *	@brief	Write byte to slave with specify register address
  * @param	Slave device address (7-bit right aligned)
  * @param	Register address
  * @param	Data byte
  * @retval	None
  ******************************************************************************
  */
void i2c_write_with_reg(uint8_t address, uint8_t reg, uint8_t data)
{
	i2c_start();
	i2c_address_direction(address << 1, I2C_Direction_Transmitter);
	i2c_transmit(reg);
	i2c_transmit(data);
	i2c_stop();
}

/**
  ******************************************************************************
  *	@brief	Write bytes to slave without specify register address where to
  *					start write
  * @param	Slave device address (7-bit right aligned)
  * @param	Pointer to data byte array
  * @param	Number of bytes to write
  * @retval	None
  ******************************************************************************
  */
void i2c_write_multi_no_reg(uint8_t address, uint8_t* data, uint8_t len)
{
	int i;
	
	i2c_start();
	i2c_address_direction(address << 1, I2C_Direction_Transmitter);
	for (i = 0; i < len; i++)
	{
		i2c_transmit(data[i]);
	}
	i2c_stop();
}

/**
  ******************************************************************************
  *	@brief	Write bytes to slave with specify register address where to
  *					start write
  * @param	Slave device address (7-bit right aligned)
  * @param	Register address where to start write
  * @param	Pointer to data byte array
  * @param	Number of bytes to write
  * @retval	None
  ******************************************************************************
  */
void i2c_write_multi_with_reg(uint8_t address, uint8_t reg, uint8_t* data, uint8_t len)
{
	int i;
	
	i2c_start();
	i2c_address_direction(address << 1, I2C_Direction_Transmitter);
	i2c_transmit(reg);
	for (i = 0; i < len; i++)
	{
		i2c_transmit(data[i]);
	}
	i2c_stop();
}

/**
  ******************************************************************************
  *	@brief	Read byte from slave without specify register address
  * @param	Slave device address (7-bit right aligned)
  * @param	Pointer to data byte to store data from slave
  * @retval	None
  ******************************************************************************
  */
void i2c_read_no_reg(uint8_t address, uint8_t* data)
{
	i2c_start();
	i2c_address_direction(address << 1, I2C_Direction_Receiver);
	*data = i2c_receive_nack();
	i2c_stop();
}

/**
  ******************************************************************************
  *	@brief	Read byte from slave with specify register address
  * @param	Slave device address (7-bit right aligned)
  * @param	Register address
  * @param	Pointer to data byte to store data from slave
  * @retval	None
  ******************************************************************************
  */
void i2c_read_with_reg(uint8_t address, uint8_t reg, uint8_t* data)
{
	i2c_start();
	i2c_address_direction(address << 1, I2C_Direction_Transmitter);
	i2c_transmit(reg);
	i2c_stop();
	i2c_start();
	i2c_address_direction(address << 1, I2C_Direction_Receiver);
	*data = i2c_receive_nack();
	i2c_stop();
}

/**
  ******************************************************************************
  *	@brief	Read bytes from slave without specify register address
  * @param	Slave device address (7-bit right aligned)
  * @param	Number of data bytes to read from slave 
  * @param	Pointer to data array byte to store data from slave
  * @retval	None
  ******************************************************************************
  */
void i2c_read_multi_no_reg(uint8_t address, uint8_t len, uint8_t* data)
{
	int i;
	
	i2c_start();
	i2c_address_direction(address << 1, I2C_Direction_Receiver);
	for (i = 0; i < len; i++)
	{
		if (i == (len - 1))
		{
			data[i] = i2c_receive_nack();
		}
		else
		{
			data[i] = i2c_receive_ack();
		}
	}
	i2c_stop();
}

/**
  ******************************************************************************
  *	@brief	Read bytes from slave with specify register address
  * @param	Slave device address (7-bit right aligned)
  * @param	Register address
  * @param	Number of data bytes to read from slave 
  * @param	Pointer to data array byte to store data from slave
  * @retval	None
  ******************************************************************************
  */
void i2c_read_multi_with_reg(uint8_t address, uint8_t reg, uint8_t len, uint8_t* data)
{
	int i;
	
	i2c_start();
	i2c_address_direction(address << 1, I2C_Direction_Transmitter);
	i2c_transmit(reg);
	i2c_stop();
	i2c_start();
	i2c_address_direction(address << 1, I2C_Direction_Receiver);
	for (i = 0; i < len; i++)
	{
		if (i == (len - 1))
		{
			data[i] = i2c_receive_nack();
		}
		else
		{
			data[i] = i2c_receive_ack();
		}
	}
	i2c_stop();
}

/** Private functions ------------------------------------------------------- */
/**
  ******************************************************************************
  *	@brief	Generate I2C start condition
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void i2c_start()
{
	// Wait until I2Cx is not busy anymore
	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
	
	// Generate start condition
	I2C_GenerateSTART(I2Cx, ENABLE);
	
	// Wait for I2C EV5. 
	// It means that the start condition has been correctly released 
	// on the I2C bus (the bus is free, no other devices is communicating))
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
}

/**
  ******************************************************************************
  *	@brief	Generate I2C stop condition
  * @param	None
  * @retval	None
  ******************************************************************************
  */
void i2c_stop()
{
	// Generate I2C stop condition
	I2C_GenerateSTOP(I2Cx, ENABLE);
}

/**
  ******************************************************************************
  *	@brief	Write slave address to I2C bus
	* @param	Slave address
	* @param	I2C direction (transmitter or receiver)
  * @retval	None
  ******************************************************************************
  */
void i2c_address_direction(uint8_t address, uint8_t direction)
{
	// Send slave address
	I2C_Send7bitAddress(I2Cx, address, direction);
	
	// Wait for I2C EV6
	// It means that a slave acknowledges his address
	if (direction == I2C_Direction_Transmitter)
	{
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	}
	else if (direction == I2C_Direction_Receiver)
	{	
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	}
}

/**
  ******************************************************************************
  *	@brief	Transmit one byte to I2C bus
  * @param	Data byte to transmit
  * @retval	None
  ******************************************************************************
  */
void i2c_transmit(uint8_t byte)
{
	// Send data byte
	I2C_SendData(I2Cx, byte);
	// Wait for I2C EV8_2.
	// It means that the data has been physically shifted out and 
	// output on the bus)
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

/**
  ******************************************************************************
  *	@brief	Receive data byte from I2C bus, then return ACK
  * @param	None
  * @retval	Received data byte
  ******************************************************************************
  */
uint8_t i2c_receive_ack()
{
	// Enable ACK of received data
	I2C_AcknowledgeConfig(I2Cx, ENABLE);
	// Wait for I2C EV7
	// It means that the data has been received in I2C data register
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));
	
	// Read and return data byte from I2C data register
	return I2C_ReceiveData(I2Cx);
}

/**
  ******************************************************************************
  *	@brief	Receive data byte from I2C bus, then return NACK
  * @param	None
  * @retval	Received data byte
  ******************************************************************************
  */
uint8_t i2c_receive_nack()
{
	// Disable ACK of received data
	I2C_AcknowledgeConfig(I2Cx, DISABLE);
	// Wait for I2C EV7
	// It means that the data has been received in I2C data register
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));
	
	// Read and return data byte from I2C data register
	return I2C_ReceiveData(I2Cx);
}

/********************************* END OF FILE ********************************/
/******************************************************************************/
