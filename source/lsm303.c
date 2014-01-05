//  LSM303 Driver
//  9/4/2011 - PDS

#include "lsm303.h"
#include "hardware.h"
#include "LPC17xx.h"

#include "lpc17xx_i2c.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"

int LSM303Init( void )
{
	PINSEL_CFG_Type PinCfg;

	//Turn on 1.8V regulator
	GPIO_SetDir(2, 0x00000002, 1);		//Regulator enable is P2.1
	GPIO_SetValue(2, 0x00000002);		//P2.1 is high to enable

	//Setup Magnetometer
	LSM303WriteReg(LSM303_MAGNETOMETER, LSM303_MAGNETOMETER_CRA_REG, LSM303_MAGNETOMETER_DR_15HZ);
	LSM303WriteReg(LSM303_MAGNETOMETER, LSM303_MAGNETOMETER_CRB_REG, LSM303_MAGNETOMETER_GAIN_1_9G);
	
	//Setup Accelerometer
	LSM303WriteReg(LSM303_ACCELEROMETER, LSM303_ACCELEROMETER_CTRL_REG2, 0x00);
	LSM303WriteReg(LSM303_ACCELEROMETER, LSM303_ACCELEROMETER_CTRL_REG3, 0x02);
	LSM303WriteReg(LSM303_ACCELEROMETER, LSM303_ACCELEROMETER_CTRL_REG4, 0x80);
	//LSM303WriteReg(LSM303_ACCELEROMETER, LSM303_ACCELEROMETER_CTRL_REG1, 0x27);//(LSM303_ACCELEROMETER_NORMAL_MODE)|(LSM303_ACCELEROMETER_ODR_50HZ)|(LSM303_ACCELEROMETER_XON)|(LSM303_ACCELEROMETER_YON)|(LSM303_ACCELEROMETER_ZON));
	
	//Setup pin P2.8 for magnetometer interrupts
	PinCfg.OpenDrain 	= PINSEL_PINMODE_NORMAL;	//Not open drain
	PinCfg.Pinmode 		= PINSEL_PINMODE_PULLDOWN;	//Interrupt is open drain
	PinCfg.Funcnum 		= PINSEL_FUNC_0;
	PinCfg.Pinnum 		= 8;
	PinCfg.Portnum 		= 2;
	PINSEL_ConfigPin(&PinCfg);
	
	//Setup P0.16 for accelerometer interrputs
	PinCfg.OpenDrain 	= PINSEL_PINMODE_NORMAL;	//Not open drain
	PinCfg.Pinmode 		= PINSEL_PINMODE_TRISTATE;	//Interrupt is push-pull
	PinCfg.Funcnum 		= PINSEL_FUNC_0;
	PinCfg.Pinnum 		= 16;
	PinCfg.Portnum 		= 0;
	PINSEL_ConfigPin(&PinCfg);
	
	
	return 0;
}

uint8_t LSM303WriteReg(LSM303Device LSM303_DEV, uint8_t RegToWrite, uint8_t ValueToWrite)
{
	I2C_M_SETUP_Type LSM303_I2C;
	
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
	LSM303_I2C.rx_data = LSM303ReadBuffer;
	LSM303_I2C.rx_length = 0;
	LSM303_I2C.retransmissions_max = 3;

	if (I2C_MasterTransferData(I2CDEV, &LSM303_I2C, I2C_TRANSFER_POLLING) == SUCCESS){
		return (0);
	} else {
		return (-1);
	}
}


uint8_t LSM303ReadReg(LSM303Device LSM303_DEV, uint8_t RegToRead)
{
	I2C_M_SETUP_Type LSM303_I2C;
	
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
	LSM303WriteBuffer[0] = RegToRead;
	
	LSM303_I2C.tx_data = LSM303WriteBuffer;
	LSM303_I2C.tx_length = 1;
	LSM303_I2C.rx_data = LSM303ReadBuffer;
	LSM303_I2C.rx_length = 1;
	LSM303_I2C.retransmissions_max = 3;

	if (I2C_MasterTransferData(I2CDEV, &LSM303_I2C, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		return (0);
	} 
	else 
	{
		LED(1,1);
		return (5);
	}
}




int LSM303SendCommand(int LSM303_DEV, int command)
{
	I2C_M_SETUP_Type LSM303_I2C;
	
	if(LSM303_DEV == 0)
	{
		LSM303_I2C.sl_addr7bit = LSM303_ACCELEROMETER_ADDRESS;
		LSM303WriteBuffer[0] = 0x20;
	}
	else if (LSM303_DEV == 1)
	{
		LSM303_I2C.sl_addr7bit = LSM303_MAGNETOMETER_ADDRESS;
		LSM303WriteBuffer[0] = 0x0F;
	}
	
	LSM303_I2C.tx_data = LSM303WriteBuffer;
	LSM303_I2C.tx_length = 1;
	LSM303_I2C.rx_data = LSM303ReadBuffer;
	LSM303_I2C.rx_length = 1;
	LSM303_I2C.retransmissions_max = 3;

	
	
	if (I2C_MasterTransferData(I2CDEV, &LSM303_I2C, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		return (0);
	} 
	else
	{	
		LED(1,1);
		return (5);
	}
}


uint8_t LSM303ReadData(LSM303Device LSM303_DEV)
{
	I2C_M_SETUP_Type LSM303_I2C;
	
	if(LSM303_DEV == LSM303_ACCELEROMETER)
	{
		LSM303_I2C.sl_addr7bit = LSM303_ACCELEROMETER_ADDRESS;
		LSM303WriteBuffer[0] = (LSM303_ACCELEROMETER_OUT_X_L) | 0x80;
	}
	else if (LSM303_DEV == LSM303_MAGNETOMETER)
	{
		LSM303_I2C.sl_addr7bit = LSM303_MAGNETOMETER_ADDRESS;
		LSM303WriteBuffer[0] = (LSM303_MAGNETOMETER_OUT_X_H) | 0x80;
	}
	
	LSM303_I2C.tx_data = LSM303WriteBuffer;
	LSM303_I2C.tx_length = 1;
	LSM303_I2C.rx_data = LSM303ReadBuffer;
	LSM303_I2C.rx_length = 6;
	LSM303_I2C.retransmissions_max = 3;

	if (I2C_MasterTransferData(I2CDEV, &LSM303_I2C, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		return (0);
	}
	else
	{
		LED(1,1);
		return (5);
	}
}
