//  LSM303 Driver
//  9/4/2011 - PDS

#include "lsm303.h"
#include "hardware.h"
#include "LPC17xx.h"

#include "lpc17xx_i2c.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"

void LSM303Init( void )
{
	PINSEL_CFG_Type PinCfg;

	//Turn on 1.8V regulator
	GPIO_SetDir(2, 0x00000002, 1);		//Regulator enable is P2.1
	GPIO_SetValue(2, 0x00000002);		//P2.1 is high to enable

	//Setup Magnetometer
	LSM303WriteReg(LSM303_MAGNETOMETER, LSM303_MAGNETOMETER_CRA_REG, LSM303_MAGNETOMETER_DR_15HZ);
	LSM303WriteReg(LSM303_MAGNETOMETER, LSM303_MAGNETOMETER_CRB_REG, LSM303_MAGNETOMETER_GAIN_1_9G);

	//Setup pin P2.8 for DRDY interrupts
	PinCfg.OpenDrain 	= PINSEL_PINMODE_NORMAL;	//Not open drain
	PinCfg.Pinmode 		= PINSEL_PINMODE_PULLDOWN;	//
	PinCfg.Funcnum 		= PINSEL_FUNC_0;
	PinCfg.Pinnum 		= 8;
	PinCfg.Portnum 		= 2;
	PINSEL_ConfigPin(&PinCfg);
	
	return;
}

Bool LSM303WriteReg(LSM303Device LSM303_DEV, uint8_t RegToWrite, uint8_t ValueToWrite)
{
	I2C_M_SETUP_Type LSM303_I2C;
	uint8_t LSM303WriteBuffer[2];
	//uint8_t LSM303ReadBuffer;
	
	if(LSM303_DEV == LSM303_ACCELEROMETER)
	{
		LSM303_I2C.sl_addr7bit = LSM303_ACCELEROMETER_ADDRESS;
	}
	else if (LSM303_DEV == LSM303_MAGNETOMETER)
	{
		LSM303_I2C.sl_addr7bit = LSM303_MAGNETOMETER_ADDRESS;
	}
	LSM303WriteBuffer[0] = RegToWrite;
	LSM303WriteBuffer[1] = ValueToWrite;
	
	LSM303_I2C.tx_data = LSM303WriteBuffer;
	LSM303_I2C.tx_length = 2;
	//LSM303_I2C.rx_data = LSM303ReadBuffer;
	LSM303_I2C.rx_length = 0;
	LSM303_I2C.retransmissions_max = 3;

	if (I2C_MasterTransferData(I2CDEV, &LSM303_I2C, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		return TRUE;
	}
	return FALSE;
}


Bool LSM303ReadReg(LSM303Device LSM303_DEV, uint8_t RegToRead, uint8_t *RegData)
{
	I2C_M_SETUP_Type LSM303_I2C;
	uint8_t LSM303WriteBuffer;
	uint8_t LSM303ReadBuffer;
	
	if(LSM303_DEV == LSM303_ACCELEROMETER)
	{
		LSM303_I2C.sl_addr7bit = LSM303_ACCELEROMETER_ADDRESS;
		//LSM303WriteBuffer[0] = 0x20;
	}
	else if (LSM303_DEV == LSM303_MAGNETOMETER)
	{
		LSM303_I2C.sl_addr7bit = LSM303_MAGNETOMETER_ADDRESS;
		//LSM303WriteBuffer[0] = 0x0F;
	}
	LSM303WriteBuffer = RegToRead;
	
	LSM303_I2C.tx_data = &LSM303WriteBuffer;
	LSM303_I2C.tx_length = 1;
	LSM303_I2C.rx_data = &LSM303ReadBuffer;
	LSM303_I2C.rx_length = 1;
	LSM303_I2C.retransmissions_max = 3;

	if (I2C_MasterTransferData(I2CDEV, &LSM303_I2C, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		*RegData = LSM303ReadBuffer;
		return TRUE;
	}
	return FALSE;
}

Bool LSM303ReadData(LSM303Device LSM303_DEV)
{
	I2C_M_SETUP_Type LSM303_I2C;
	uint8_t LSM303WriteBuffer;
	uint8_t LSM303ReadBuffer[6];
	
	if(LSM303_DEV == LSM303_ACCELEROMETER)
	{
		LSM303_I2C.sl_addr7bit = LSM303_ACCELEROMETER_ADDRESS;
		LSM303WriteBuffer = LSM303_ACCELEROMETER_OUT_X_L;
	}
	else if (LSM303_DEV == LSM303_MAGNETOMETER)
	{
		LSM303_I2C.sl_addr7bit = LSM303_MAGNETOMETER_ADDRESS;
		LSM303WriteBuffer = LSM303_MAGNETOMETER_OUT_X_H;
	}
	
	LSM303_I2C.tx_data = &LSM303WriteBuffer;
	LSM303_I2C.tx_length = 1;
	LSM303_I2C.rx_data = LSM303ReadBuffer;
	LSM303_I2C.rx_length = 6;
	LSM303_I2C.retransmissions_max = 3;

	if (I2C_MasterTransferData(I2CDEV, &LSM303_I2C, I2C_TRANSFER_POLLING) != SUCCESS)
	{
		return FALSE;
	} 
	
	if(LSM303_DEV == LSM303_ACCELEROMETER)
	{
		LSM303AccelerometerData[0] = LSM303ReadBuffer[0] || (LSM303ReadBuffer[1] << 8);
		LSM303AccelerometerData[1] = LSM303ReadBuffer[2] || (LSM303ReadBuffer[3] << 8);
		LSM303AccelerometerData[2] = LSM303ReadBuffer[4] || (LSM303ReadBuffer[5] << 8);
	}
	else if (LSM303_DEV == LSM303_MAGNETOMETER)
	{
		LSM303MagnetometerData[0] = LSM303ReadBuffer[1] || (LSM303ReadBuffer[0] << 8);
		LSM303MagnetometerData[1] = LSM303ReadBuffer[3] || (LSM303ReadBuffer[2] << 8);
		LSM303MagnetometerData[2] = LSM303ReadBuffer[5] || (LSM303ReadBuffer[4] << 8);
	}
	return TRUE;
}
