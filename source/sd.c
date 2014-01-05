//  SD Card Hardware Interface for LPC17XX
//  9/4/2011 - PDS

#include "hardware.h"
#include "sd.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_gpio.h"

//SSP Configuration
SSP_CFG_Type SSP_ConfigStruct;

int SDHwInit( void )
{
	// initialize SSP configuration structure to default
	SSP_ConfigStructInit(&SSP_ConfigStruct);
	
	//SSP_ConfigStruct.ClockRate = 100000;
	
	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP0, &SSP_ConfigStruct);

	// Enable SSP peripheral
	SSP_Cmd(LPC_SSP0, ENABLE);
	
	SSP_SetClock (LPC_SSP0, 100000);
	
	return 0;
}

//SD Card Chip select
//	1 - Select
// 	0 - Deselect
void SDChipSelect (int select)
{
	if (select == 0)
	{
		GPIO_SetValue(1, (1<<25));
	}
	else if (select == 1)
	{
		GPIO_ClearValue(1, (1<<25));
	}
	return;
}


int SDSendByte (int ByteToSend)
{
	unsigned char bytesToSend[4];
	unsigned char bytesToRecieve[4];
	SSP_DATA_SETUP_Type xferConfig;
	
	bytesToSend[0] = 0x01;
	bytesToSend[1] = 0x02;
	bytesToSend[2] = 0x03;
	bytesToSend[3] = 0x04;
	
	bytesToRecieve[0] = 0x05;
	bytesToRecieve[1] = 0x06;
	bytesToRecieve[2] = 0x07;
	bytesToRecieve[3] = 0x08;
	

	xferConfig.tx_data = bytesToSend;
	xferConfig.rx_data = bytesToRecieve;
	xferConfig.length = 4;
	
	SDChipSelect(1);
	SSP_ReadWrite(LPC_SSP0, &xferConfig, SSP_TRANSFER_POLLING);
	SDChipSelect(0);
	
	return 0;
}
