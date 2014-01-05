


//All headers are deinfed here
#include "main.h"

//Set debug to 1 to enable debug commands
// DBG must be defined to output commands based on the hardware used
// DBG uses the standard printf formatting
#define _SDCARD_DEBUG	1

#if _SDCARD_DEBUG == 1
	#define DBG VCOM_printf
#else
	#define DBG(x ...) do { } while (0)
#endif

//Read and write timouts, these are used instead of the TAAC, NSAC, and R2W_FACTOR
#define SDCARD_READ_TIMEOUT_MS					200
#define SDCARD_WRITE_TIMEOUT_MS					600

#define SDCARD_MAX_TRANSFER_FREQUENCY_HZ		25000000

static const SDCard_Command_t SDCard_Commands [CMD_END] =
{
	{ 0x40,	SDCard_R1 },		//CMD0
	{ 0x41,	SDCard_R1 },		//CMD1
	{ 0x48,	SDCard_R7 },		//CMD8
	{ 0x49,	SDCard_R1_Data },	//CMD9
	{ 0x4A,	SDCard_R1_Data },	//CMD10
	{ 0x4C,	SDCard_R1 },		//CMD12
	{ 0x4D,	SDCard_R2 },		//CMD13
	{ 0x50,	SDCard_R1 },		//CMD16
	{ 0x51,	SDCard_R1_Data },	//CMD17
	{ 0x52,	SDCard_R1_Data },	//CMD18
	{ 0x58,	SDCard_R1_Data },	//CMD24
	{ 0x59,	SDCard_R1_Data },	//CMD25
	{ 0x5B,	SDCard_R1 },		//CMD27
	{ 0x5C,	SDCard_R1b},		//CMD28
	{ 0x5D,	SDCard_R1b},		//CMD29
	{ 0x5E,	SDCard_R1 },		//CMD30
	{ 0x60,	SDCard_R1 },		//CMD32
	{ 0x61,	SDCard_R1 },		//CMD33
	{ 0x62,	SDCard_R1 },		//CMD34
	{ 0x63,	SDCard_R1 },		//CMD35
	{ 0x64,	SDCard_R1 },		//CMD36
	{ 0x65,	SDCard_R1 },		//CMD37
	{ 0x66,	SDCard_R1b},		//CMD38
	{ 0x6A,	SDCard_R1b},		//CMD42
	{ 0x77,	SDCard_R1 },		//CMD55
	{ 0x78,	SDCard_R1 },		//CMD56
	{ 0x7A,	SDCard_R3 },		//CMD58
	{ 0x7B,	SDCard_R1 },		//CMD59
	{ 0x69,	SDCard_R1 }			//ACMD41
};







//-------------------------------------------------------------------------------
//Hardware Abstraction Layer
//-------------------------------------------------------------------------------
void SDHAL_HardwareInit(void);
void SDHAL_Select(uint8_t sel);
uint8_t SDHAL_SendByte(uint8_t ByteToSend);
uint32_t SDHAL_SetFrequency(uint32_t FrequencyToSet);
void SDHAL_SendBlock(const uint8_t *DataToSend, uint32_t BytesToSend);
void SDHAL_ReceiveBlock(const uint8_t *ReceivedData, uint32_t BytesToGet);
Bool SDHAL_CardPresent (void);
Bool SDHAL_CardWriteProtected (void);
uint8_t SDHAL_WaitReady (void);
void SDHAL_DelayMS (uint32_t delay);

//Perform any hardware specific initalization
//	SSP, CS lines, etc...
void SDHAL_HardwareInit(void)
{
	//For LPC1758, pin initalizaion is performed in hardware.c
	//SD card is on SSP0
	SSP_CFG_Type SSPConfig;
	PINSEL_CFG_Type PinCfg;
	
	PinCfg.OpenDrain 	= PINSEL_PINMODE_NORMAL;		//Not open drain
	PinCfg.Pinmode 		= PINSEL_PINMODE_PULLDOWN;		//Enable pull down resistor
	PinCfg.Funcnum 		= PINSEL_FUNC_0;				//Set as GPIO
	PinCfg.Pinnum 		= 22;
	PinCfg.Portnum 		= 0;
	PINSEL_ConfigPin(&PinCfg);
	
	GPIO_SetDir(0, (1<<22), 1);		//Setup card detect pin (p0.22) as input
	
	//-------------------------------------------------------------
	//  SSP Initialization
	//	P1.20 - SCK
	//	P1.23 - MISO
	//	P1.24 - MOSI
	//	P1.25 - CS (GPIO Mode)
	//-------------------------------------------------------------
	
	//Setup Pins
	GPIO_SetDir(1, (1<<25), 1);		//Setup CS pin as output
	GPIO_SetValue(1, (1<<25));		//CS is high
	
	//SCK0
	PinCfg.Funcnum 		= PINSEL_FUNC_3;
	PinCfg.OpenDrain 	= PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode 		= PINSEL_PINMODE_TRISTATE;
	//PinCfg.Pinmode 		= PINSEL_PINMODE_PULLDOWN;
	PinCfg.Portnum 		= 1;
	PinCfg.Pinnum 		= 20;
	PINSEL_ConfigPin(&PinCfg);
	
	//MISO0
	//PinCfg.Pinmode 		= PINSEL_PINMODE_TRISTATE;
	PinCfg.Pinnum 		= 23;
	PINSEL_ConfigPin(&PinCfg);
	
	//MOSI0
	//PinCfg.Pinmode 		= PINSEL_PINMODE_PULLDOWN;
	PinCfg.Pinnum 		= 24;
	PINSEL_ConfigPin(&PinCfg);
	
	SSP_ConfigStructInit(&SSPConfig);
	SSPConfig.ClockRate = SDCARD_IDENTIFICATION_MODE_CLOCK_RATE_HZ;		//Hz
	SSP_Init(LPC_SSP0, &SSPConfig);
	SSP_Cmd(LPC_SSP0, ENABLE);
	
	return;
}

//Select chip
// sel = 1 should pull the chip select line low
// sel = 0 should pull the chip select line high
void SDHAL_Select(uint8_t sel)
{
	if(sel == 1)
	{
		GPIO_ClearValue(1, (1<<25));	
	}
	else
	{
		GPIO_SetValue(1, (1<<25));	
	}
	return;
}

//Send byte over SPI/SSP
// Send one byte to the device over SPI/SSP
// Returns the byte read while sending
// This function waits untill a byte is recieved before returning
uint8_t SDHAL_SendByte(uint8_t ByteToSend)
{
	uint8_t RXData;
	
	SSP_DATA_SETUP_Type SSPData;
	SSPData.tx_data = &ByteToSend;
	SSPData.rx_data = &RXData;
	SSPData.length = 1;

	SSP_ReadWrite(LPC_SSP0, &SSPData, SSP_TRANSFER_POLLING);
	return RXData;
}


//TODO: Fix the config struct init to allow other modes...
//Set communication frequency of the SSP/SPI bus
// Frequency is given in Hz
uint32_t SDHAL_SetFrequency(uint32_t FrequencyToSet)
{
	SSP_CFG_Type SSPConfig;
	
	SSP_Cmd(LPC_SSP0, DISABLE);
	SSP_ConfigStructInit(&SSPConfig);
	SSPConfig.ClockRate = FrequencyToSet;		//Hz
	SSP_Init(LPC_SSP0, &SSPConfig);
	SSP_Cmd(LPC_SSP0, ENABLE);
	
	return SSPConfig.ClockRate;
}

//Send a block of data to the SD card over SPI/SSP
//This would work well as DMA i think...
void SDHAL_SendBlock(const uint8_t *DataToSend, uint32_t BytesToSend)
{
	SSP_DATA_SETUP_Type SSPData;
	SSPData.tx_data = DataToSend;
	SSPData.rx_data = NULL;
	SSPData.length = BytesToSend;

	SSP_ReadWrite(LPC_SSP0, &SSPData, SSP_TRANSFER_POLLING);
	return;
}

//Receive a block of data to the SD card over SPI/SSP
void SDHAL_ReceiveBlock(const uint8_t *ReceivedData, uint32_t BytesToGet)
{
	SSP_DATA_SETUP_Type SSPData;
	SSPData.tx_data = NULL;
	SSPData.rx_data = ReceivedData;
	SSPData.length = BytesToGet;

	SSP_ReadWrite(LPC_SSP0, &SSPData, SSP_TRANSFER_POLLING);
	return;
}

//Returns TRUE if a card is present
Bool SDHAL_CardPresent (void)
{
	uint32_t PortStatus;
	PortStatus = GPIO_ReadValue(0);
	if((PortStatus & (1<<22)) == (1<<22))
	{
		return TRUE;
	}
	return FALSE;
}

//Returns 1 if a card is write proteted or not present
Bool SDHAL_CardWriteProtected (void)
{
	//This function is not used, always return false
	return FALSE;
}

//Wait for card to initalize
//Returns response from the card
//TODO: This function should have a ~500ms timout
uint8_t SDHAL_WaitReady (void)
{
	uint8_t res;

	res = SDHAL_SendByte(0xFF);
	while (res != 0xFF)
	{
		res = SDHAL_SendByte(0xFF);
	}

	return res;
}

//
//
void SDHAL_DelayMS (uint32_t DelayTime)
{
	Delay(DelayTime);
	return;
}

//-------------------------------------------------------------------------------

//test SD card functions
uint8_t SDTest (uint8_t in)
{
	SSP_DATA_SETUP_Type xferConfig;
	uint8_t Tx_Buf[4];
	uint8_t Rx_Buf[4];


	switch (in)
	{
		case 0:
			SDHAL_HardwareInit();
			return 0;
			
		case 1:
			return SDHAL_CardPresent();
			
		case 2:
			SDHAL_Select(1);
			return 0;
			
		case 3:
			SDHAL_Select(0);
			return 0;
			
		case 4:
			SDHAL_HardwareInit();		//Initalize Hardware
			Delay(100);
			Tx_Buf[0] = 0x00;
			Tx_Buf[1] = 0xF0;
			Tx_Buf[2] = 0xAA;
			Tx_Buf[3] = 0x24;
			
			
			xferConfig.tx_data = Tx_Buf;
			xferConfig.rx_data = Rx_Buf;
			xferConfig.length = 4;
			
			SDHAL_Select(1);
			
			SSP_ReadWrite(LPC_SSP0, &xferConfig, SSP_TRANSFER_POLLING);
			
			
			//SDHAL_SendByte(0xaa);
			SDHAL_Select(0);
			return 0;
			
		case 5:
			SDHAL_SetFrequency (SDCARD_IDENTIFICATION_MODE_CLOCK_RATE_HZ);
			return SDHAL_SendByte(0xFA);
			
		case 6:
			SDCard_Init();
			return 0;
		
		default:
			return 0;
	}
}

/** Send command to the SD card.
*		
* \param[in]	commandIndex The command to send.
* \param[in]	arg The argument data to send.
* \return 		The data from the card. The return data depends on the response code of the command.
*/
uint32_t SDCard_SendCmd (SDCard_CommandIndex_t commandIndex, uint32_t arg)
{
	uint32_t ch = 0xFF;
	uint32_t i;
	uint32_t busy;

	//Chip Select
	SDHAL_Select(1);

	//Send command code
	SDHAL_SendByte (SDCard_Commands[commandIndex].txData);

	//Send command's arguments
	SDHAL_SendByte (arg >> 24);
	SDHAL_SendByte (arg >> 16);
	SDHAL_SendByte (arg >> 8);
	SDHAL_SendByte (arg);

	//Send CRC (only CMD0 and CMD8 need a valid CRC)
	if (commandIndex == CMD0)
	{
		SDHAL_SendByte (0x95);
	}
	else if (commandIndex == CMD8)
	{
		SDHAL_SendByte (0x87);
	}
	else
    {
		SDHAL_SendByte (0xFF);
	}

	//Wait for response
	for (i = 9; i && (ch == 0xFF); --i)
	{
		ch = SDHAL_SendByte (0xFF);
	}
	
	//No response from card
	if (i == 0)
	{
		SDHAL_Select(0);
		return (0xFFFFFFFF);
	}

	switch (SDCard_Commands[commandIndex].resp)
	{
		case SDCard_R1:
			SDHAL_Select(0);
			return (ch);
			
		//Same as a normal R1 response, but don't deselect the chip so that the data can be sent/recieved
		case SDCard_R1_Data:
			return (ch);
			
		case SDCard_R1b:
			//Wait for end of busy tokens
			for (i = SDCard_Status.writeTimeoutBytes; i; i--)
			{
				busy = SDHAL_SendByte(0xFF);
				if(busy != SDCARD_BUSY_TOKEN)
				{
					//Card is no longer busy
					SDHAL_Select(0);
					return (0x00000000 | ch);
				}
			}
				
			//Card is still busy
			SDHAL_Select(0);
			return (0xFFFFFF00 | ch);
			
		case SDCard_R2:
			arg  = (((uint32_t) ch << 8) & 0x0000FF00);
			arg |= (SDHAL_SendByte(0xFF) & 0xFF);
			SDHAL_Select(0);
			return (arg);
			
		//Response to CMD58
		//Returns OCR register (32 bits)
		case SDCard_R3 :
			arg  = (((uint32_t) SDHAL_SendByte (0xFF) << 24) & 0xFF000000);
			arg |= (((uint32_t) SDHAL_SendByte (0xFF) << 16) & 0x00FF0000);
			arg |= (((uint32_t) SDHAL_SendByte (0xFF) << 8 ) & 0x0000FF00);
			arg |= (SDHAL_SendByte (0xFF) & 0xFF);
			SDHAL_Select(0);
			return (arg);
			
		//Response to CMD8, (this only works when card is in 'idle' mode)
		case SDCard_R7:
			if(ch == SDCARD_IDLE)								//Command accepted, card is SD v2.0 Compliant
			{
				ch = ((uint32_t) SDHAL_SendByte (0xFF) << 24) & 0xFF000000;
				ch |= ((uint32_t) SDHAL_SendByte (0xFF) << 16) & 0x00FF0000;
				ch |= ((uint32_t) SDHAL_SendByte (0xFF) << 8) & 0x0000FF00;
				ch |= SDHAL_SendByte (0xFF) & 0xFF;
				SDHAL_Select(0);
				return ch;
			}

			else if(ch == (SDCARD_IDLE | SDCARD_ILLEGAL_CMD))	//Illegal command, card is not SD v2.0 Compliant
			{
				SDHAL_Select(0);
				return (0x00000000 | ch);
			}
			
			else												//Other error, not sure what to do if this happens
			{
				SDHAL_Select(0);
				return (0xFFFFFF00 | ch);
			}
			
		default:
			DBG("Unhandled response\n");
			arg  = ((uint32_t) ch << 24) & 0xFF000000;
			arg |= ((uint32_t) SDHAL_SendByte (0xFF) << 16) & 0x00FF0000;
			arg |= ((uint32_t) SDHAL_SendByte (0xFF) << 8 ) & 0x0000FF00;
			arg |= SDHAL_SendByte (0xFF) & 0xFF;
			SDHAL_Select(0);
			return (arg);
	}
}



/** Read register from the card
*
* \param[in]	Register The command to send.
* \param[in]	*pData Pointer to the variable to store the data
* \return 		SDCard_State variable indicating the status of the command.
*/
SDCard_State_t SDCard_ReadRegister(uint8_t Register, uint8_t *pData)
{
	uint8_t i;
	uint8_t res;
	
	SDHAL_Select(0);
	SDHAL_SendByte(0xFF);
	
	if(Register == SDCARD_REGISTER_CSD)
	{
		//Send CMD9 to get CSD
		res = SDCard_SendCmd(CMD9, 0);
	}
	else if(Register == SDCARD_REGISTER_CID)
	{
		//Send CMD10 to get CID
		res = SDCard_SendCmd(CMD10, 0);
	}
	else
	{
		DBG("Acess to that register is not implemented\n");
		return SDCard_ParameterError;
	}

	if (res == SDCARD_OK)
	{
		//Wait for start-of-data token
		for (i = 8; i; --i)
		{
			res = SDHAL_SendByte(0xFF);
			
			if (res == SDCARD_START_BLOCK_TOKEN)
			{
				for (i = 0; i < 16; ++i)
				{
					*pData++ = SDHAL_SendByte(0xFF);
				}
				
				//CRC
				SDHAL_SendByte(0xFF);
				SDHAL_SendByte(0xFF);
				SDHAL_Select(0);
				return (SDCard_Ok);
			}
			else if((res <= SDCARD_DATA_ERROR_TOKEN_MASK) && (res > 0))		//Data error token
			{
				SDHAL_Select(0);
				res &= SDCARD_DATA_ERROR_TOKEN_MASK;
				if(res == SDCARD_DATA_ERROR_TOKEN_RANGE)
				{
					return SDCard_ParameterError;
				}
				else if(res == SDCARD_DATA_ERROR_TOKEN_ECC)
				{
					return SDCard_DataError;
				}
				else if(res == SDCARD_DATA_ERROR_TOKEN_CC)
				{
					return SDCard_CardError;
				}
				else
				{
					return SDCard_OtherError;
				}
			}
		}
	}
	
	//Card did not respond
	SDHAL_Select(0);
	return SDCard_NoResponse;
}

/** Set the transfer speed of the card.
*		This function should be used to set the transfer frequency as it also modifies the timeouts properly
* \param[in]	TransferSpeed The transfer speed to set (in Hz).
* \return 		TRUE if transfer speed is valid.
*/
Bool SDCard_SetTransferSpeed(uint32_t TransferSpeed)
{
	if(TransferSpeed <= SDCARD_MAX_TRANSFER_FREQUENCY_HZ)
	{
		SDCard_Status.readTimeoutBytes = (SDCARD_READ_TIMEOUT_MS*(TransferSpeed/1000))/8;
		SDCard_Status.writeTimeoutBytes = (SDCARD_WRITE_TIMEOUT_MS*(TransferSpeed/1000))/8;;
		SDHAL_SetFrequency(TransferSpeed);
		return TRUE;
	}
	return FALSE;
}

/** Read data from the card
*
* \param[in]	*pData Pointer to the variable to store the data.
* \param[in]	address The block number to read.
* \param[in]	length	The number of blocks to read.
* \return 		SDCard_State variable indicating the status of the command.
*/
SDCard_State_t SDCard_Read(uint8_t *pData, uint32_t address, uint32_t length)
{
	uint32_t res;
	uint32_t i;

	//Make sure the card is initalized
	if(SDCard_Status.cardInitalized != TRUE)
	{
		return SDCard_NotPresent;
	}

	//For SDSC, the address should be in bytes, for SDHC and SDXC, the address is in blocks (512 bytes)
	if((SDCard_Status.diskType == DISKTYPE_SDv1) || (SDCard_Status.diskType == DISKTYPE_SDv2))
	{
		address *= SDCard_Status.blockSize;
	}
	
	//For synchronization
	SDHAL_Select (0);
	SDHAL_SendByte (0xFF);
	
	//Send read command
	if(length == 1)		//Single block read
	{
		res = SDCard_SendCmd(CMD17, address);
	}
	else				//Multiple block read
	{
		res = SDCard_SendCmd(CMD18, address);
	}
	
	if (res == SDCARD_OK)		//Card responded correctly, wait for data token
	{
		res = SDHAL_SendByte(0xFF);
		for (i = SDCard_Status.readTimeoutBytes; i; i--)
		{
			//Data start token
			if(res == SDCARD_START_BLOCK_TOKEN)
			{
				break;
			}
			
			//Data error token
			else if((res <= SDCARD_DATA_ERROR_TOKEN_MASK) && (res > 0))
			{
				SDHAL_Select(0);
				res &= SDCARD_DATA_ERROR_TOKEN_MASK;
				if(res == SDCARD_DATA_ERROR_TOKEN_RANGE)
				{
					return SDCard_ParameterError;
				}
				else if(res == SDCARD_DATA_ERROR_TOKEN_ECC)
				{
					return SDCard_DataError;
				}
				else if(res == SDCARD_DATA_ERROR_TOKEN_CC)
				{
					return SDCard_CardError;
				}
				else
				{
					return SDCard_OtherError;
				}
			}
			
			res = SDHAL_SendByte(0xFF);
		}
		
		if(i==0)		//Card did not respond with data token after Tnac tries
		{
			SDHAL_Select (0);
			return (SDCard_CardError);
		}
		
		for(i=0;i<length;i++)		//Recieve data, if multiple block read, send CMD12 to stop.
		{
			SDHAL_ReceiveBlock (pData, SDCard_Status.blockSize);
			SDHAL_SendByte (0xFF);		//CRC bit
			SDHAL_SendByte (0xFF);		//CRC bit
		}
		if (length > 1)
		{
			res = SDCard_SendCmd(CMD12,0x00);
		}
		SDHAL_Select (0);
		SDHAL_SendByte (0xFF);
		return (SDCard_Ok);
	}
	
	//Card did not respond to read command correctly
	SDHAL_Select (0);
	return (SDCard_CardError);
}

/** Sends CMD13 to get the status of the card
*
* \return 		The two-bit status register of the card.
*/
uint16_t SDCard_GetStatus(void)
{
	uint16_t res = 0;
	res = (uint16_t)SDCard_SendCmd(CMD13,0x00);
	//SDHAL_Select (0);
	//SDHAL_SendByte (0xFF);
	return res;
}


/** Write data to the card
*
* \param[in]	*pData Pointer to the data to write.
* \param[in]	address The block number to read.
* \param[in]	length	The number of blocks to read.
* \return 		SDCard_State variable indicating the status of the command.
*/
SDCard_State_t SDCard_Write(const uint8_t *pData, uint32_t address, uint32_t length)
{
	uint32_t res;
	uint32_t i;
	uint8_t StartToken;

	//Make sure the card is initalized
	if(SDCard_Status.cardInitalized != TRUE)
	{
		return SDCard_NotPresent;
	}
	
	//Make sure the card is not write protected
	if(SDCard_Status.writeProtect > 0)
	{
		return SDCard_OtherError;
	}

	//For SDSC, the address should be in bytes, for SDHC and SDXC, the address is in blocks (512 bytes)
	if((SDCard_Status.diskType == DISKTYPE_SDv1) || (SDCard_Status.diskType == DISKTYPE_SDv2))
	{
		address *= SDCard_Status.blockSize;
	}

	//  For synchronization
	SDHAL_Select (0);
	SDHAL_SendByte (0xFF);

	if (length == 1)	//Single block write
	{
		StartToken = SDCARD_START_BLOCK_TOKEN;
		res = SDCard_SendCmd (CMD24, address);
	}
	else				//multiple block write
	{
		StartToken = SDCARD_START_MULTI_BLOCK_WRITE_TOKEN;
		res = SDCard_SendCmd (CMD25, address);
	}
	
	if (res == SDCARD_OK)
	{
		SDHAL_SendByte (0xFF);
		SDHAL_SendByte (StartToken);
		for(i=0; i<length; i++)
		{
			SDHAL_SendBlock(pData, length);
			//  CRC Send
			SDHAL_SendByte (0xFF);
			SDHAL_SendByte (0xFF);
			
			res = SDHAL_SendByte (0xFF);
			
			//Card did not return OK after writing block
			if ((res & SDCARD_DATA_RESPONSE_TOKEN_MASK) != SDCARD_DATA_RESPONSE_TOKEN_OK)
			{
				if((res & SDCARD_DATA_RESPONSE_TOKEN_MASK) == SDCARD_DATA_RESPONSE_TOKEN_CRC_ERROR)
				{
					return SDCard_DataError;
				}
				else
				{
					return SDCard_CardError;
				}
			}
			
			//Wait for card to finish writing
			for (i = SDCard_Status.writeTimeoutBytes; i; i--)
			{
				if (SDHAL_SendByte(StartToken) != 0x00)
				{
					break;
				}
			}
			if (i == 0)		//Card did not finish writing
			{
				return (SDCard_NoResponse);
			}
		}
		
		//Send stop transmission token
		if(length > 1)
		{
			SDHAL_SendByte (SDCARD_STOP_TRANS_TOKEN);
		}
		
		SDHAL_Select (0);
		SDHAL_SendByte (0xFF);
		return (SDCard_Ok);
	}

	SDHAL_Select(0);
	return (SDCard_CardError);
}

/** Verify data written to the card
*
* \param[in]	*pData Pointer to the data to to verify against.
* \param[in]	address The block number to start the comparison.
* \param[in]	length	The number of blocks to compare.
* \return 		SDCard_State variable indicating the status of the command.
*/
SDCard_State_t SDCard_Verify(uint8_t *pData, uint32_t address, uint32_t length)
{
	uint32_t res;
	uint32_t i;
	uint32_t j;

	//Make sure the card is initalized
	if(SDCard_Status.cardInitalized != TRUE)
	{
		return SDCard_NotPresent;
	}

	//For SDSC, the address should be in bytes, for SDHC and SDXC, the address is in blocks (512 bytes)
	if((SDCard_Status.diskType == DISKTYPE_SDv1) || (SDCard_Status.diskType == DISKTYPE_SDv2))
	{
		address *= SDCard_Status.blockSize;
	}

	//For synchronization
	SDHAL_Select (0);
	SDHAL_SendByte (0xFF);

	//Send read command
	if(length == 1)		//Single block read
	{
		res = SDCard_SendCmd(CMD17, address);
	}
	else				//Multiple block read
	{
		res = SDCard_SendCmd(CMD18, address);
	}
	
	if (res == SDCARD_OK)		//Card responded correctly, wait for data token
	{
		res = SDHAL_SendByte(0xFF);
		for (i = SDCard_Status.readTimeoutBytes; i; i--)
		{
			//Data start token
			if(res == SDCARD_START_BLOCK_TOKEN)
			{
				break;
			}
			
			//Data error token
			else if((res <= SDCARD_DATA_ERROR_TOKEN_MASK) && (res > 0))
			{
				SDHAL_Select(0);
				res &= SDCARD_DATA_ERROR_TOKEN_MASK;
				if(res == SDCARD_DATA_ERROR_TOKEN_RANGE)
				{
					return SDCard_ParameterError;
				}
				else if(res == SDCARD_DATA_ERROR_TOKEN_ECC)
				{
					return SDCard_DataError;
				}
				else if(res == SDCARD_DATA_ERROR_TOKEN_CC)
				{
					return SDCard_CardError;
				}
				else
				{
					return SDCard_OtherError;
				}
			}
			
			res = SDHAL_SendByte(0xFF);
		}
		
		if(i == 0)		//Card did not respond with data token
		{
			SDHAL_Select (0);
			return (SDCard_CardError);
		}

		//Start token received, start comparing data
		for(j=length; j; j--)
		{
			//Recieve a block of data, compare it to pData
			for(i=0; i<SDCard_Status.blockSize; i++)		
			{
				res = SDHAL_SendByte(0xFF);
				if((res ^ *pData) != 0x00)
				{
					return SDCard_MisCompare;
				}
				pData++;
			}
			
			//Receive CRC bytes
			SDHAL_SendByte (0xFF);
			SDHAL_SendByte (0xFF);
		}
		
		if (length > 1)
		{
			res = SDCard_SendCmd(CMD12, 0x00);
		}
		
		SDHAL_Select (0);
		SDHAL_SendByte (0xFF);
		return (SDCard_Ok);
	}
	
	//Card did not respond to read command correctly
	SDHAL_Select (0);
	return (SDCard_CardError);
}




/** Initalizes the SD card
*
* \return 		SDCard_State variable indicating the status of the command.
*/
SDCard_State_t SDCard_Init (void)
{
	uint32_t i;
	uint32_t res;
	uint32_t HCS = 0;
	uint16_t C_SIZE = 0;
	uint16_t C_SIZE_MULTI = 0;
	uint16_t READ_BL_LEN = 0;
	uint32_t MULT = 0;
	uint32_t BLOCK_LEN = 0;
	uint32_t BLOCKNR = 0;
	uint32_t SIZE = 0;
	
	uint8_t SDCard_CSD[16];

	//Initalize card status
	SDCard_Status.CSD_Structure		= 0;
	SDCard_Status.blockSize			= 0;
	SDCard_Status.diskType			= DISKTYPE_UNKNOWN;
	SDCard_Status.writeProtect		= 0;
	SDCard_Status.cardPresent		= FALSE;
	SDCard_Status.sizeKB			= 0;
	SDCard_Status.cardInitalized	= FALSE;

	//Initalize SPI/SSP and any other hardware specific stuFF
	DBG("Starting SD card initalization\n");
	DBG("Initalizing hardware\n");
	SDHAL_HardwareInit();
	
	//Check if card is present
	if ( !SDHAL_CardPresent() )
	{
		DBG("Memory Card not detected, initalization failed.\n");
		return (SDCard_NotPresent);
	}
	SDCard_Status.cardPresent = TRUE;

	//Clock frequency must be <= 400kHz for initalizaion
	SDCard_SetTransferSpeed(SDCARD_IDENTIFICATION_MODE_CLOCK_RATE_HZ);
	
	DBG("Waiting 80 clock cycles...\n");
	//After power up at least 74 clock cycles with the card deselected are required prior to starting bus communication
	
	for (i = 0; i < 10; i++)
	{
		SDHAL_Select (0);
		for (res = 10; res; --res)
		{
			SDHAL_SendByte (0xFF);
		}
	
		//CMD0 (Go to IDLE) to put SD card in SPI mode
		res = SDCard_SendCmd (CMD0, 0);

		if (res == SDCARD_IDLE)
		{
			break;
		}
	}

	if (res != SDCARD_IDLE)
	{
		DBG ("res = %d\n",res);
		DBG ("Memory card failed to enter idle mode, initalization failed.\n");
		return (SDCard_NoResponse);
	}
  
	DBG ("Memory card entered idle mode with CMD0.\n");
	//SDCard_Status.diskType = DISKTYPE_UNKNOWN;			//Initalize card type, check this later.
	
	//send command 8 to comply with SD v2.0+
	res = SDCard_SendCmd (CMD8, (0x100 | SDCARD_CMD8_TEST_CODE));
	if(res == (SDCARD_IDLE | SDCARD_ILLEGAL_CMD))		
	{
		//Card is not SD V2.0 compliant
		DBG("CMD8 rejected, card is not SDv2 compliant\n",res);
		SDCard_Status.diskType = DISKTYPE_SDv1;
	}
	else if (res == (0x100 | SDCARD_CMD8_TEST_CODE))
	{
		SDCard_Status.diskType = DISKTYPE_SDv2;			//Card is SD V2.0 Compliant
		HCS = 1<<30;									//Enable high capacity functions for ACMD41
	}
	else
	{
		DBG("Invalid response to CMD8 test code (%d)\n",res);
		return (SDCard_CardError);
	}
  
	//Send CMD58 to determine allowable power modes
	for (i = 10; i; --i)
	{
		res = SDCard_SendCmd (CMD58, 0);
		if (res != 0xFFFFFFFF)							//Valid response to CMD58
		{
			DBG("Valid response to CMD58 (0x%8X)\n",res);
			break;
		}
	}
	
	if (i == 0)
	{
		DBG ("Memory card did not respond to CMD58");
	}
	else
	{
		if(res & 0x80)
		{
			DBG("Dual voltage card detected: 0x%8X\n",res);
		}
		if(res != 0xFF8000)	//Card does not have standard voltage range
		{
			DBG("Card has non-standard voltage range: 0x%8X\n",res);
		}
		else
		{
			DBG("Voltage range: 2.7-3.6V\n");
		}
	}

	//Send ACMD41 to start initalization and set HCS bit if nessecary
	for (i = 100; i; --i)
	{
		SDHAL_SendByte (0xFF);
		res = SDCard_SendCmd (CMD55, 0);
		SDHAL_SendByte (0xFF);
		res = SDCard_SendCmd (ACMD41, HCS);
		
		if (res == SDCARD_OK)
		{
			break;
		}
		
		SDHAL_DelayMS (50);
	}

	if (i == 0)
	{
		DBG ("Card did not respond to ACMD41, initalization failed.\n");
		return (SDCard_NoResponse);
	}
	
	//Check the CCS bit in the OCR register if SD v2.0 card is detected
	if(SDCard_Status.diskType == DISKTYPE_SDv2)
	{
		//Get OCR
		for (i = 10; i; --i)
		{
			res = SDCard_SendCmd (CMD58, 0);
			if (res != 0xFFFFFFFF)				//Valid response to CMD58
			{
				break;
			}
		}
	
		if(i > 0)
		{
			if( (res & (1<<30)) == (1<<30) )	//CCS bit is set, card is SDHC/SDXC
			{
				SDCard_Status.diskType = DISKTYPE_SDv2HC;
			}
		}
		else
		{
			DBG ("Memory card did not respond to CMD58");
			return (SDCard_NoResponse);			//No response to CMD58
		}
	
	}
	
	// Read CSD.  CSD must always be valid
	res = SDCard_ReadRegister(SDCARD_REGISTER_CSD, SDCard_CSD);
	if (res != SDCard_Ok)
	{
		DBG ("Error reading CSD (%d)\n", res);
		return (res);
	}
	
#if _SDCARD_DEBUG == 1
	for (i = 0; i<16; i++)
	{
		if(i==0)
		{
			DBG("CSD: 0x%02X",SDCard_CSD[i]);
		}
		else
		{
			DBG(" 0x%02X ",SDCard_CSD[i]);
		}
	}
	DBG("\n");
#endif
	
	//Process CSD data, set block size, and setup status structure
	SDCard_Status.CSD_Structure = ((SDCard_CSD [0] & 0xC0) >> 6) + 1;		//Determine CSD version
	
	if(SDCard_Status.CSD_Structure == 1)			//CSD version 1
	{
		C_SIZE = ((SDCard_CSD[6] & 0x03) << 10) | (SDCard_CSD[7] << 2) | ((SDCard_CSD[8] & 0xC0) >> 6);
		C_SIZE_MULTI = ((SDCard_CSD[9] & 0x03) << 1) | ((SDCard_CSD[10] & 0x80) >> 7);
		READ_BL_LEN = SDCard_CSD[5] & 0x0F;
		BLOCK_LEN = 1<<READ_BL_LEN;
		MULT = 1<<(C_SIZE_MULTI+2);
		BLOCKNR = (C_SIZE+1)*MULT;
		SIZE = BLOCKNR*BLOCK_LEN;
		
		SDCard_Status.blockSize = BLOCK_LEN;
		SDCard_Status.sizeKB = BLOCKNR*BLOCK_LEN/1024;
		
		//Setup the block size, we always want to use 512 bytes
		if(SDCard_Status.blockSize != 512)
		{
			DBG("Block length is too long, setting to 512\n");
			res = SDCard_SendCmd(CMD16, 512);
			SDCard_Status.blockSize = 512;
		}
	}
	else if(SDCard_Status.CSD_Structure == 2)		//CSD version 2
	{
		C_SIZE = SDCard_CSD[9]|(SDCard_CSD[8]<<8)|(SDCard_CSD[7]<<16);
		
		if(C_SIZE >= 0x00FFFF)
		{
			SDCard_Status.diskType = DISKTYPE_SDv2XC;
		}
		
		SDCard_Status.blockSize = 512;
		SDCard_Status.sizeKB = (C_SIZE+1)*512;
	}
	else
	{
		DBG("Invalid CSD version\n");
		return (SDCard_CardError);
	}
	
	//Check for write protection
	if(SDHAL_CardWriteProtected ())			//Hardware write protection
	{
		SDCard_Status.writeProtect |= 0x01;
	}
	
	if((SDCard_CSD[14] & 0x10 >> 4))		//Software temporary write protection
	{
		SDCard_Status.writeProtect |= 0x02;
	}
	
	if((SDCard_CSD[14] & 0x20 >> 5))		//Software permanent write protection
	{
		SDCard_Status.writeProtect |= 0x04;
	}
	
	//Check for max speed, and set to 25MHz if possible
	if(SDCard_CSD[3] == 0x32)
	{
		DBG("Setting clock speed to 25MHz\n");
		SDCard_SetTransferSpeed(SDCARD_MAX_TRANSFER_FREQUENCY_HZ);
	}
	else
	{
		DBG("Max speed not recognized (%d)\n",SDCard_CSD[3]);
	}
	
	
#if _SDCARD_DEBUG == 1
	DBG("\n--------------------------------------\n");
	DBG("Card Information:\n");
	DBG("--------------------------------------\n");
	DBG("Card Type: ");
	switch (SDCard_Status.diskType)
	{
		case DISKTYPE_SDv1:
			DBG("SDv1\n");
			break;
		
		case DISKTYPE_SDv2:
			DBG("SDv2 SC\n");
			break;
			
		case DISKTYPE_SDv2HC:
			DBG("SDv2 HC\n");
			break;
			
		case DISKTYPE_SDv2XC:
			DBG("SDv2 XC\n");
			break;
			
		case DISKTYPE_UNKNOWN:
			DBG("Unknown\n");
			break;
	}

	DBG("Size: %dkB or %d.%dGB\n",SDCard_Status.sizeKB,SDCard_Status.sizeKB/1024/1024, (((SDCard_Status.sizeKB/1024)*100)/1024)-100);
	DBG("CSD Version: %d\n", SDCard_Status.CSD_Structure);
	DBG("Block Size: %d\n", SDCard_Status.blockSize);
	
	if(SDCard_Status.writeProtect == 0x00)
	{
		DBG("Card is not write protected\n");
	}
	else
	{
		if(SDCard_Status.writeProtect & 0x01)
		{
			DBG("Hardware write protection on\n");
		}
		if(SDCard_Status.writeProtect & 0x02)
		{
			DBG("Software temporary write protection on\n");
		}
		if(SDCard_Status.writeProtect & 0x04)
		{
			DBG("Software permanent write protection on\n");
		}
	}
	DBG("--------------------------------------\n\n");
#endif
	
	DBG ("Initalization completed successfully\n");
	SDCard_Status.cardInitalized = TRUE;
	return (SDCard_Ok);
}
