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

	GPSDataReady = FALSE;
	GPS_Pos = 0;
	GPS_Stat = 0;
	GPS_Term = 0;

	//Initialize UART2 pins
	Chip_IOCON_PinMux(LPC_IOCON, 0, 10, IOCON_MODE_INACT, IOCON_FUNC1);//Set up TXD pins
	Chip_IOCON_PinMux(LPC_IOCON, 0, 11, IOCON_MODE_INACT, IOCON_FUNC1);//Set up RXD pins

	//Setup pins as output
	Chip_GPIO_WriteDirBit(LPC_GPIO, 0, 10, true); //Set TXD to output
	Chip_GPIO_WriteDirBit(LPC_GPIO, 0, 11, false); //set RXD to input

	/* Setup UART for 115.2K8N1 */
	Chip_UART_Init(LPC_UART2);
	Chip_UART_SetBaud(LPC_UART2, 9600);
	Chip_UART_ConfigData(LPC_UART2, UART_DATABIT_8, UART_PARITY_NONE, UART_STOPBIT_1);

	///* Enable UART Transmit */
	//Chip_UART_TxCmd(LPC_USART2, ENABLE);




//	//Enable interrupts for UART2
//    NVIC_SetPriority(UART2_IRQn, 0x1E); //((0x01<<3)|0x01));
//    NVIC_EnableIRQ(UART2_IRQn);

	GPSqueue = xQueueCreate(2, sizeof(char));//create queue for storing incoming GPS strings
	Chip_UART_IntConfig(LPC_UART2, UART_INTCFG_RBR, ENABLE); //turns on interrupt when byte is received from UART2
	NVIC_EnableIRQ(UART2_IRQn);//Enable interrupts for UART2



	//Setup GPS reset line
	Chip_IOCON_PinMux(LPC_IOCON, 1, 29, IOCON_MODE_INACT, IOCON_FUNC0);//Set up pins
	Chip_GPIO_WriteDirBit(LPC_GPIO, 1, 29 , true); //GPS reset pin is P1.29 (set to output)
	GPSReset(1);

	/* About a 3Hz on/off toggle rate */
	vTaskDelay(configTICK_RATE_HZ / 6);

	GPSReset(0);


	return;
}



//P1.29 is reset line, low resets the GPS
void GPSReset (uint8_t val)
{
	if(val == 1)		//Reset the GPS
	{
		Chip_GPIO_WritePortBit(LPC_GPIO, 1, 29 , false);
	}
	else if (val == 0)	//Unreset the GPS
	{
		Chip_GPIO_WritePortBit(LPC_GPIO, 1, 29 , true);
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
			printf("------------------------------------\n");
			printf("GPS Data\n");
			printf("------------------------------------\n");
			printf("UTC Date:%d/%d/20%d\n", GPSData.UTC_MONTH, GPSData.UTC_DAY, GPSData.UTC_YEAR);
			printf("UTC Time:%d:%d:%d\n", GPSData.UTC_HR, GPSData.UTC_MIN, GPSData.UTC_SEC);
			printf("Latitude: %d:%d", GPSData.LAT_DEG, GPSData.LAT_MIN);
			if(GPSData.LAT_NORTH == TRUE)
			{
				printf(" N\n");
			}
			else
			{
				printf(" S\n");
			}
			printf("Longitude: %d:%d", GPSData.LONG_DEG, GPSData.LONG_MIN);
			if(GPSData.LONG_EAST == TRUE)
			{
				printf(" E\n");
			}
			else
			{
				printf(" W\n");
			}
			printf("Altitude: %d\n", GPSData.ALT);
			printf("Satellites used: %d\n", GPSData.SATS);
			printf("------------------------------------\n");
		}
		else
		{
			printf("No GPS Lock\n");
		}
	}

	return TRUE;
}


void UART2_IRQHandler(void)
{
	//handles interrupts for UART0
	uint8_t charRecieved;

	Chip_UART_ReceiveByte(LPC_UART2 ,&charRecieved);//read a character from the UART0 input buffer

	xQueueSendFromISR(GPSqueue, &charRecieved,NULL);//put a recieved character on the queue

	Chip_UART_IntGetStatus(LPC_UART2);

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


void ParseString(char inputstring[100], char delimiter, char *items)
{
	uint8_t n, m;
	uint8_t itemcount=0;
	uint8_t nstart=0;
	uint8_t nend=0;

	for(n=0;n<100;n++)
	{
		if ((inputstring[n] == delimiter)|(n-nstart>10))
		{
			nend = n;
			//copy contents between delimiters to the items array
			for(m=0;m<nend-nstart-1;m++)
			{
				//*items[itemcount][m] = inputstring[m+nstart+1];
			}
			itemcount++;
			nstart = n;
		}

	}

	return;

}





/* UART2 (or output) thread */
portTASK_FUNCTION(vGPSTask, pvParameters) {

	uint8_t datareceived=0;
	char GPSBuffer[75];

	uint8_t i, j;
	uint8_t rxdata;
	uint8_t datacount;

	GPSInit();


	datacount=0;

	//$GPVTG, 000.0,T,,M,000.0,N,0000.0,K,A*3D<CR><LF>
	//$GPRMC,111636.932,A,2447.0949,N,12100.5223,E,000.0,000.0,030407,,,A*61<CR><LF>
	//$GPGSV,3,3,12,14,39,330,42,01,06,299,38,31,30,256,44,32,36,320,47*7B<CR><LF>
	//$GPGSA,A,3,05,12,21,22,30,09,18,06,14,01,31,,1.2,0.8,0.9*36<CR><LF>
	//$GPGLL,2447.0944,N,12100.5213,E,112609.932,A,A*57<CR><LF>
	//$GPGGA,111636.932,2447.0949,N,12100.5223,E,1,11,0.8,118.2,M,,,,0000*02<CR><LF>

	while (1)
	{

		xQueueReceive(GPSqueue, &datareceived, portMAX_DELAY); //wait indefinitely for data to appear on queue

		if (datareceived=='$')
		{
			datacount=0;
		}

		GPSBuffer[datacount] = datareceived;
		datacount++;

		printf("%c", datareceived);

//		if (datareceived == (char)10 && GPSBuffer[0] == '$')//All GPS return strings are terminated by <CR><LF>
//		{
//
//			for(i=0;i<datacount;i++)
//			{
//			printf(GPSBuffer);
//			}
//
//			datacount=0;
//
//		}


		/*
		if (datareceived == (char)10 && GPSBuffer[0] == '$')//All GPS return strings are terminated by <CR><LF>
		{


			if(GPSBuffer[4] == 'T' && GPSBuffer[5] == 'G')	//GPVTG
			{
				//GPS_Stat = ;
			}
			else if(GPSBuffer[4] == 'M' && GPSBuffer[5] == 'C')	//GPRMC
			{
				GPS_Stat = 3;
			}
			else if(GPSBuffer[4] == 'S' && GPSBuffer[5] == 'V')	//GPGSV
			{
				//GPS_Stat = 2;
			}
			else if(GPSBuffer[4] == 'S' && GPSBuffer[5] == 'A')	//GPGSA
			{
				//GPS_Stat = 2;
			}
			else if(GPSBuffer[4] == 'L' && GPSBuffer[5] == 'L')	//GPGLL
			{
				//GPS_Stat = 2;
			}
			else if(GPSBuffer[4] == 'G' && GPSBuffer[5] == 'A')	//GPGGA
			{
				//$GPGGA,111636.932,2447.0949,N,12100.5223,E,1,11,0.8,118.2,M,,,,0000*02<CR><LF>
				GPS_Stat = 2;

				//UTC Time
				GPSData.UTC_HR 	= (GPSBuffer[7] - 48)*10 + (GPSBuffer[8] - 48);
				GPSData.UTC_MIN	= (GPSBuffer[9] - 48)*10 + (GPSBuffer[10] - 48);
				GPSData.UTC_SEC	= (GPSBuffer[11] - 48)*10 + (GPSBuffer[12] - 48);
				//Latitude
				GPSData.LAT_DEG = (GPSBuffer[14] - 48)*10 + (GPSBuffer[15] - 48);
				GPSData.LAT_MIN = (GPSBuffer[16] - 48)*10 + (GPSBuffer[17] - 48);
				//Latitude N or S
				if(GPSBuffer[19] == 'N')
				{
					GPSData.LAT_NORTH = TRUE;
				}
				else
				{
					GPSData.LAT_NORTH = FALSE;
				}
				//Longitude
				GPSData.LONG_DEG = (GPSBuffer[21] - 48)*100 + (GPSBuffer[1] - 48)*10 + (GPSBuffer[22] - 48);
				GPSData.LONG_MIN = (GPSBuffer[23] - 48)*10 + (GPSBuffer[24] - 48);
				//Longitude E/W
				if(GPSBuffer[26] == 'E')
				{
					GPSData.LONG_EAST = TRUE;
					//printf("E");
				}
				else
				{
					GPSData.LONG_EAST = FALSE;
					//printf("W");
				}
				//Lock status
				GPSData.LOCK_STATUS = GPSBuffer[28] - 48;
				//Number of sattelites
				GPSData.SATS = (GPSBuffer[30] - 48)*10 + (GPSBuffer[31] - 48);
				//Horizontal dilution of precision

				//Altitude
				if(GPSBuffer[37] == '-')
				{
					GPSData.ALT = -1;
				}
				else
				{
					GPSData.ALT = GPSBuffer[37] - 48;
				}

				for(j=38;j<41;j++)
				{
					if(GPSBuffer[j] == '.')
					{
						break;
					}
					GPSData.ALT = GPSData.ALT*10 + GPSBuffer[j] - 48;
				}



			}


			else
			{
				GPS_Stat = 0;
			}

*/






/*
			for(i=0;i<datacount;i++)
			{
				switch (GPS_Stat)
				{
				case 0:		//Wait for '$'
					if((GPSData.UTC_YEAR != 0xFF) && (GPSData.UTC_HR != 0xFF))
					{
						NVIC_DisableIRQ(UART2_IRQn);
						//printf("Data collected\n");
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
							//printf("GPGGA\n");
							GPS_Stat = 2;
						}
						else if(GPSBuffer[3] == 'M' && GPSBuffer[4] == 'C')	//GPRMC
						{
							//printf("GPRMC\n");
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
							//printf("UTC:%d:%d:%d\n", GPSData.UTC_HR, GPSData.UTC_MIN, GPSData.UTC_SEC);
							break;

						case 2:			//Latitude
							GPSData.LAT_DEG = (GPSBuffer[0] - 48)*10 + (GPSBuffer[1] - 48);
							GPSData.LAT_MIN = (GPSBuffer[2] - 48)*10 + (GPSBuffer[3] - 48);
							//printf("LAT: %d:%d\n", GPSData.LAT_DEG, GPSData.LAT_MIN);
							break;

						case 3:			//Latitude N or S
							if(GPSBuffer[0] == 'N')
							{
								GPSData.LAT_NORTH = TRUE;
								//printf("N");
							}
							else
							{
								GPSData.LAT_NORTH = FALSE;
								//printf("S");
							}
							break;

						case 4:			//Longitude
							GPSData.LONG_DEG = (GPSBuffer[0] - 48)*100 + (GPSBuffer[1] - 48)*10 + (GPSBuffer[2] - 48);
							GPSData.LONG_MIN = (GPSBuffer[3] - 48)*10 + (GPSBuffer[4] - 48);
							//printf("LONG: %d:%d\n", GPSData.LONG_DEG, GPSData.LONG_MIN);
							break;

						case 5:			//Longitude E/W
							if(GPSBuffer[0] == 'E')
							{
								GPSData.LONG_EAST = TRUE;
								//printf("E");
							}
							else
							{
								GPSData.LONG_EAST = FALSE;
								//printf("W");
							}
							break;

						case 6:			//Lock status
							GPSData.LOCK_STATUS = GPSBuffer[0] - 48;
							//printf("Lock: %d\n", GPSData.LOCK_STATUS);
							break;

						case 7:			//Number of sattelites
							GPSData.SATS = (GPSBuffer[0] - 48)*10 + (GPSBuffer[1] - 48);
							//printf("Sats: %d\n", GPSData.SATS);
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
							//printf("Alt: %d\n", GPSData.ALT);
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
							//printf("%d/%d/20%d\n", GPSData.UTC_MONTH, GPSData.UTC_DAY, GPSData.UTC_YEAR);
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
		}

*/


	}

}



