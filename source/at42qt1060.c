//  AT42QT1060 Driver for LPC17XX
//  9/7/2011 - PDS

#include "main.h"

Bool AT42QT1060Init( void )
{
	uint16_t timeout;
	//Bool InitSuccess;
	uint8_t resp;
	
	timeout = 0;
	
	GPIO_SetDir(0, (1<<15), 1);		//Setup P0.15 for output as reset line
	GPIO_SetDir(0, (1<<17), 0);		//Setup P0.17 for interrupt input
	
	AT42QT1060Reset(1);				//Reset the device
	Delay (5);
	AT42QT1060Reset(0);				//Unreset the device
	
	AT42QT1060WriteByte(AT42QT1060_REG_CALIBRATE, 0x01);
	
	AT42QT1060ReadByte(AT42QT1060_REG_DETECT_STATUS, &resp);
	while((resp & 0x80) == 0x80)
	{
		timeout++;
		AT42QT1060ReadByte(AT42QT1060_REG_DETECT_STATUS, &resp);
		if(timeout > AT42QT1060_CAL_TIMEOUT)
		{
			return FALSE;
		}
	}
	
	GPIO_IntCmd(0, (1<<17), 1);		//Enable interrupt for CHG line
	return TRUE;
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
Bool AT42QT1060ReadByte(uint8_t address, uint8_t *readData)
{
	I2C_M_SETUP_Type AT42QT1060_I2C;
	uint8_t ReadByte;

	AT42QT1060_I2C.sl_addr7bit = AT42QT1060_I2C_ADDRESS;
	AT42QT1060_I2C.tx_data = &address;
	AT42QT1060_I2C.tx_length = 1;
	AT42QT1060_I2C.rx_data = &ReadByte;
	AT42QT1060_I2C.rx_length = 1;
	AT42QT1060_I2C.retransmissions_max = 3;

	if(I2C_MasterTransferData(I2CDEV, &AT42QT1060_I2C, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		*readData = ReadByte;
		return TRUE;
	}
	return FALSE;
}

Bool AT42QT1060WriteByte(uint8_t address, uint8_t ByteToWrite)
{
	I2C_M_SETUP_Type AT42QT1060_I2C;
	uint8_t WriteBuffer[2];
	
	WriteBuffer[0] = address;
	WriteBuffer[1] = ByteToWrite;
	
	AT42QT1060_I2C.sl_addr7bit = AT42QT1060_I2C_ADDRESS;
	AT42QT1060_I2C.tx_data = WriteBuffer;
	AT42QT1060_I2C.tx_length = 1;
	//AT42QT1060_I2C.rx_data = &ReadByte;
	AT42QT1060_I2C.rx_length = 0;
	AT42QT1060_I2C.retransmissions_max = 3;

	if (I2C_MasterTransferData(I2CDEV, &AT42QT1060_I2C, I2C_TRANSFER_POLLING) == SUCCESS){
		return TRUE;
	}
	
	return FALSE;
}
