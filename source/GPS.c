//GPS source file for puzzle box mainboard
//9/3/2011 - PDS


#include "main.h"

uint8_t GPSBuffer[10];
uint8_t GPS_Pos;
uint8_t GPS_Stat;
uint8_t GPS_Term;


void ClearGPSBuffer (void);

void GPSInit (void)
{
	UART_CFG_Type UARTConfigStruct;
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	PINSEL_CFG_Type PinCfg;

	GPSDataReady = FALSE;
	GPS_Pos = 0;
	GPS_Stat = 0;
	GPS_Term = 0;
	
	//Initialize UART2 pins
	PinCfg.Funcnum 		= 1;
	PinCfg.OpenDrain 	= 0;
	PinCfg.Pinmode 		= 0;
	PinCfg.Pinnum 		= 10;
	PinCfg.Portnum 		= 0;
	PINSEL_ConfigPin(&PinCfg);
	
	PinCfg.Pinnum 		= 11;
	PINSEL_ConfigPin(&PinCfg);

	//Initialize UART Configuration parameter structure to default states
	//	- Baudrate = 9600bps
	//	- 8 data bit
	//	- 1 Stop bit
	//	- No parity
	UART_ConfigStructInit(&UARTConfigStruct);
	
	//Initialize UART2
	UART_Init(LPC_UART2, &UARTConfigStruct);
	
	//Initialize FIFOConfigStruct to default states
	//	- FIFO_DMAMode = DISABLE
	//	- FIFO_Level = UART_FIFO_TRGLEV0
	//	- FIFO_ResetRxBuf = ENABLE
	//	- FIFO_ResetTxBuf = ENABLE
	//	- FIFO_State = ENABLE
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	
	UARTFIFOConfigStruct.FIFO_Level = UART_FIFO_TRGLEV3;

	// Initialize FIFO for UART2 peripheral
	UART_FIFOConfig(LPC_UART2, &UARTFIFOConfigStruct);
	
	//Enable UART Rx interrupt
	UART_IntConfig(LPC_UART2, UART_INTCFG_RBR, ENABLE);
	
	//Enable interrupts for UART2
    NVIC_SetPriority(UART2_IRQn, 0x1E); //((0x01<<3)|0x01));
    //NVIC_EnableIRQ(UART2_IRQn);
	
	//Setup GPS reset line
	GPIO_SetDir(1, (1<<29), 1);				//GPS reset pin is P1.29 (set to output)
	GPSReset(1);
	Delay(10);
	GPSReset(0);
	
	return;
}

//P1.29 is reset line, low resets the GPS
void GPSReset (uint8_t val)
{
	if(val == 1)		//Reset the GPS
	{
		GPIO_ClearValue(1, (1<<29));
	}
	else if (val == 0)	//Unreset the GPS
	{
		GPIO_SetValue(1, (1<<29));
	}
	return;
}

Bool GPSGetData(Bool toScreen)
{
	GPSData.UTC_YEAR = 0xFF;
	GPSData.UTC_HR = 0xFF;
	GPSDataReady = FALSE;
	
	NVIC_EnableIRQ(UART2_IRQn);
	
	while(GPSDataReady == FALSE)
	{
	}
	
	if(toScreen == TRUE)
	{
		if(GPSData.LOCK_STATUS > 0)
		{
			VCOM_printf("------------------------------------\n");
			VCOM_printf("GPS Data\n");
			VCOM_printf("------------------------------------\n");
			VCOM_printf("UTC Date:%d/%d/20%d\n", GPSData.UTC_MONTH, GPSData.UTC_DAY, GPSData.UTC_YEAR);
			VCOM_printf("UTC Time:%d:%d:%d\n", GPSData.UTC_HR, GPSData.UTC_MIN, GPSData.UTC_SEC);
			VCOM_printf("Latitude: %d:%d", GPSData.LAT_DEG, GPSData.LAT_MIN);
			if(GPSData.LAT_NORTH == TRUE)
			{
				VCOM_printf(" N\n");
			}
			else
			{
				VCOM_printf(" S\n");
			}
			VCOM_printf("Longitude: %d:%d", GPSData.LONG_DEG, GPSData.LONG_MIN);
			if(GPSData.LONG_EAST == TRUE)
			{
				VCOM_printf(" E\n");
			}
			else
			{
				VCOM_printf(" W\n");
			}
			VCOM_printf("Altitude: %d\n", GPSData.ALT);
			VCOM_printf("Satellites used: %d\n", GPSData.SATS);
			VCOM_printf("------------------------------------\n");
		}
		else
		{
			VCOM_printf("No GPS Lock\n");
		}
	}
	
	return TRUE;
}

void UART2_IRQHandler(void)
{
	uint8_t i, j;
	uint8_t rxdata;
	uint8_t tempBuffer[15];
	tempBuffer[14] = '\0';
	
	//Read interrupt source, this must be done to clear interrupt
	uint32_t intsrc;
	intsrc = UART_GetIntId(LPC_UART2);

	UART_Receive(LPC_UART2, tempBuffer,	14, BLOCKING);
	
	for(i=0;i<14;i++)
	{
		switch (GPS_Stat)
		{
			case 0:		//Wait for '$'
				if((GPSData.UTC_YEAR != 0xFF) && (GPSData.UTC_HR != 0xFF))
				{
					NVIC_DisableIRQ(UART2_IRQn);
					//VCOM_printf("Data collected\n");
					GPSDataReady = TRUE;
					return;
				}
				if(tempBuffer[i] == '$')
				{
					GPS_Term = 0;
					GPS_Stat = 1;
				}
				break;
				
			case 1:		//Identify string
				GPSBuffer[GPS_Pos] = tempBuffer[i];
				GPS_Pos++;
				if(GPS_Pos > 4)
				{
					if(GPSBuffer[3] == 'G' && GPSBuffer[4] == 'A')	//GPGGA
					{
						//VCOM_printf("GPGGA\n");
						GPS_Stat = 2;
					}
					else if(GPSBuffer[3] == 'M' && GPSBuffer[4] == 'C')	//GPRMC
					{
						//VCOM_printf("GPRMC\n");
						GPS_Stat = 3;
					}
					else
					{
						GPS_Stat = 0;
					}
					GPS_Pos = 0;
				}
				break;
				
			case 2:		//GPGGA string
				if(tempBuffer[i] == ',')
				{
					switch (GPS_Term)
					{
						case 1:			//UTC Time
							GPSData.UTC_HR 	= (GPSBuffer[0] - 48)*10 + (GPSBuffer[1] - 48);
							GPSData.UTC_MIN	= (GPSBuffer[2] - 48)*10 + (GPSBuffer[3] - 48);
							GPSData.UTC_SEC	= (GPSBuffer[4] - 48)*10 + (GPSBuffer[5] - 48);
							//VCOM_printf("UTC:%d:%d:%d\n", GPSData.UTC_HR, GPSData.UTC_MIN, GPSData.UTC_SEC);
							break;
							
						case 2:			//Latitude
							GPSData.LAT_DEG = (GPSBuffer[0] - 48)*10 + (GPSBuffer[1] - 48);
							GPSData.LAT_MIN = (GPSBuffer[2] - 48)*10 + (GPSBuffer[3] - 48);
							//VCOM_printf("LAT: %d:%d\n", GPSData.LAT_DEG, GPSData.LAT_MIN);
							break;
						
						case 3:			//Latitude N or S
							if(GPSBuffer[0] == 'N')
							{
								GPSData.LAT_NORTH = TRUE;
								//VCOM_printf("N");
							}
							else
							{
								GPSData.LAT_NORTH = FALSE;
								//VCOM_printf("S");
							}
							break;
							
						case 4:			//Longitude
							GPSData.LONG_DEG = (GPSBuffer[0] - 48)*100 + (GPSBuffer[1] - 48)*10 + (GPSBuffer[2] - 48);
							GPSData.LONG_MIN = (GPSBuffer[3] - 48)*10 + (GPSBuffer[4] - 48);
							//VCOM_printf("LONG: %d:%d\n", GPSData.LONG_DEG, GPSData.LONG_MIN);
							break;
							
						case 5:			//Longitude E/W
							if(GPSBuffer[0] == 'E')
							{
								GPSData.LONG_EAST = TRUE;
								//VCOM_printf("E");
							}
							else
							{
								GPSData.LONG_EAST = FALSE;
								//VCOM_printf("W");
							}
							break;
						
						case 6:			//Lock status
							GPSData.LOCK_STATUS = GPSBuffer[0] - 48;
							//VCOM_printf("Lock: %d\n", GPSData.LOCK_STATUS);
							break;
							
						case 7:			//Number of sattelites
							GPSData.SATS = (GPSBuffer[0] - 48)*10 + (GPSBuffer[1] - 48);
							//VCOM_printf("Sats: %d\n", GPSData.SATS);
							break;
							
						case 9:			//Altitude
							if(GPSBuffer[0] == '-')
							{
								GPSData.ALT = -1;
							}
							else
							{
								GPSData.ALT = GPSBuffer[0] - 48;
							}
							
							for(j=1;j<10;j++)
							{
								if(GPSBuffer[j] == '.')
								{
									break;
								}
								GPSData.ALT = GPSData.ALT*10 + GPSBuffer[j] - 48;
							}
							//VCOM_printf("Alt: %d\n", GPSData.ALT);
							GPS_Stat = 0;		//This ends data collection for this header
							break;
					}
					
					ClearGPSBuffer();
					GPS_Term++;
					GPS_Pos = 0;
				}
				else
				{
					GPSBuffer[GPS_Pos] = tempBuffer[i];
					GPS_Pos++;
				}
				break;
				
				
			case 3:		//GPRMC string
				if(tempBuffer[i] == ',')
				{
					switch (GPS_Term)
					{
						case 9:			//UTC Date
							GPSData.UTC_YEAR	= (GPSBuffer[4] - 48)*10 + (GPSBuffer[5] - 48);
							GPSData.UTC_MONTH	= (GPSBuffer[2] - 48)*10 + (GPSBuffer[3] - 48);
							GPSData.UTC_DAY		= (GPSBuffer[0] - 48)*10 + (GPSBuffer[1] - 48);
							//VCOM_printf("%d/%d/20%d\n", GPSData.UTC_MONTH, GPSData.UTC_DAY, GPSData.UTC_YEAR);
							GPS_Stat = 0;		//This ends data collection for this header
							break;
					}
					
					ClearGPSBuffer();
					GPS_Term++;
					GPS_Pos = 0;
				}
				else
				{
					GPSBuffer[GPS_Pos] = tempBuffer[i];
					GPS_Pos++;
				}
				break;
		}
	}
	return;
}


void ClearGPSBuffer (void)
{
	GPSBuffer[0] = '\0';
	GPSBuffer[1] = '\0';
	GPSBuffer[2] = '\0';
	GPSBuffer[3] = '\0';
	GPSBuffer[4] = '\0';
	GPSBuffer[5] = '\0';
	GPSBuffer[6] = '\0';
	GPSBuffer[7] = '\0';
	GPSBuffer[8] = '\0';
	GPSBuffer[9] = '\0';
}
