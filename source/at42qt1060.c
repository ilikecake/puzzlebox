//  AT42QT1060 Driver for LPC17XX
//  9/7/2011 - PDS

#include "at42qt1060.h"

#include "hardware.h"
#include "LPC17xx.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_gpio.h"

void AT42QT1060Init( void )
{
	GPIO_SetDir(0, (1<<15), 1);
	GPIO_SetValue(0, (1<<15));
}

//Reset or unreset the AT42QT1060
//reset is on line P0.15 and has an external pull up.
//	reset = 1 - reset device
//	reset = 0 - unreset device
void AT42QT1060Reset(unsigned char reset)
{
	if (reset == 0)			//Set P0.15 high (not reset)
	{
		GPIO_SetValue(0, (1<<15));
	}
	else if (reset == 1)	//P0.15 is low (reset)
	{
		GPIO_ClearValue(0, (1<<15));
	}
	return;
}

//Read a byte from the AT42QT1060
unsigned char AT42QT1060ReadByte(unsigned char address)
{
	I2C_M_SETUP_Type AT42QT1060_I2C;
	unsigned char ReadByte;

	AT42QT1060_I2C.sl_addr7bit = AT42QT1060_I2C_ADDRESS;
	AT42QT1060_I2C.tx_data = &address;
	AT42QT1060_I2C.tx_length = 1;
	AT42QT1060_I2C.rx_data = &ReadByte;
	AT42QT1060_I2C.rx_length = 1;
	AT42QT1060_I2C.retransmissions_max = 3;

	if(I2C_MasterTransferData(I2CDEV, &AT42QT1060_I2C, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		return ReadByte;
	}
	return 0;
}

unsigned char AT42QT1060WriteByte(unsigned char address, unsigned char ByteToWrite)
{
	I2C_M_SETUP_Type AT42QT1060_I2C;
	unsigned char WriteBuffer[2];
	
	WriteBuffer[0] = address;
	WriteBuffer[1] = ByteToWrite;
	

	AT42QT1060_I2C.sl_addr7bit = AT42QT1060_I2C_ADDRESS;
	AT42QT1060_I2C.tx_data = WriteBuffer;
	AT42QT1060_I2C.tx_length = 1;
	//AT42QT1060_I2C.rx_data = &ReadByte;
	AT42QT1060_I2C.rx_length = 0;
	AT42QT1060_I2C.retransmissions_max = 3;

	if (I2C_MasterTransferData(I2CDEV, &AT42QT1060_I2C, I2C_TRANSFER_POLLING) == SUCCESS){
		return (0);
	} else {
		return (-1);
	}
	return 0;
}
