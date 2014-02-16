//GPS source file for puzzle box mainboard
//9/3/2011 - PDS



#include "main.h"



void GPSInit (void)
{

	GPSDataReady = FALSE;
	GPS_Pos = 0;
	GPS_Term = 0;
	GPS_State=0;
	gps_datacount=0;

	//Initialize UART2 pins
	Chip_IOCON_PinMux(LPC_IOCON, 0, 10, IOCON_MODE_INACT, IOCON_FUNC1);//Set up TXD pins
	Chip_IOCON_PinMux(LPC_IOCON, 0, 11, IOCON_MODE_INACT, IOCON_FUNC1);//Set up RXD pins

	//Setup pins
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

	GPSqueue = xQueueCreate(4, sizeof(char));//create queue for storing incoming GPS strings
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



Bool GPSGetData(uint8_t forceDisplay)
{

	if (GPSData.LOCK_STATUS > 0 || forceDisplay == 1)
	{
		printf("------------------------------------\r\n");
		printf("GPS Data\r\n");
		printf("------------------------------------\r\n");
		printf("UTC Date:%u/%u/20%u\r\n", GPSData.UTC_MONTH, GPSData.UTC_DAY, GPSData.UTC_YEAR);
		printf("UTC Time:%u:%u:%f\r\n", GPSData.UTC_HR, GPSData.UTC_MIN, GPSData.UTC_SEC);
		printf("Latitude: %u:%f", GPSData.LAT_DEG, GPSData.LAT_MIN);
		if(GPSData.LAT_NORTH == TRUE)
		{
			printf(" N\r\n");
		}
		else
		{
			printf(" S\r\n");
		}
		printf("Longitude: %u:%f", GPSData.LONG_DEG, GPSData.LONG_MIN);
		if(GPSData.LONG_EAST == TRUE)
		{
			printf(" E\r\n");
		}
		else
		{
			printf(" W\r\n");
		}
		printf("Altitude: %d\r\n", GPSData.ALT);
		printf("Satellites used: %u\r\n", GPSData.SATS);
		printf("------------------------------------\r\n");

//			printf("\r\nTime: %u:%u:%f",GPSData.UTC_HR, GPSData.UTC_MIN, GPSData.UTC_SEC);
//			printf(", Latitude: %udeg %fmin",GPSData.LAT_DEG, GPSData.LAT_MIN);
//			if(GPSData.LAT_NORTH == TRUE)
//			{
//				printf(" N");
//			}
//			else
//			{
//				printf(" S");
//			}
//			printf(", Longitude: %udeg %fmin",GPSData, GPSData.LONG_DEG, GPSData.LONG_MIN);
//			if(GPSData.LONG_EAST == TRUE)
//			{
//				printf(" E");
//			}
//			else
//			{
//				printf(" W");
//			}
//			printf(", Satellites: %u", GPSData.SATS);
//			printf(", Altitude: %f\r\n", GPSData.ALT);


	}
	else
	{
		printf("No GPS Lock\r\n");
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








void ParseGPSdata(uint8_t gps_datareceived)
{
	uint8_t j;

	if (gps_datareceived=='$')
			{
				gps_datacount=0;
			}

			GPSBuffer[gps_datacount] = gps_datareceived;
			//printf("%c",GPSBuffer[datacount]);

			if (GPSBuffer[gps_datacount] == ',')//Identify type of data string     //All GPS return strings are terminated by <CR><LF>
			{

				if (GPSBuffer[0] == '$')//Identify type of data string     //All GPS return strings are terminated by <CR><LF>
				{

					if(GPSBuffer[4] == 'T' && GPSBuffer[5] == 'G')	//GPVTG
					{
						GPS_State = 1;
					}
					else if(GPSBuffer[4] == 'M' && GPSBuffer[5] == 'C')	//GPRMC
					{
						GPS_State = 2;
					}
					else if(GPSBuffer[4] == 'S' && GPSBuffer[5] == 'V')	//GPGSV
					{
						GPS_State = 3;
					}
					else if(GPSBuffer[4] == 'S' && GPSBuffer[5] == 'A')	//GPGSA
					{
						GPS_State = 4;
					}
					else if(GPSBuffer[4] == 'L' && GPSBuffer[5] == 'L')	//GPGLL
					{
						GPS_State = 5;
					}
					else if(GPSBuffer[4] == 'G' && GPSBuffer[5] == 'A')	//GPGGA
					{
						GPS_State = 6;
					}
					else
					{
						GPS_State = 0;
					}
					GPS_Term=0;


				}
				else //parse a piece of the GPS output
				{
					GPS_Term++;

	//				printf("(");
	//				for(j=0;j<=datacount;j++)
	//				{
	//					printf("%c",GPSBuffer[j]);
	//				}
	//				printf(")");


					if (GPS_State==1)
					{//$GPVTG, 000.0,T,,M,000.0,N,0000.0,K,A*3D<CR><LF>
						if (GPS_Term==1)
						{ //Course
						}
						else if (GPS_Term==2)
						{ //Speed over ground (knots)
						}
						else if (GPS_Term==2)
						{ //Speed over ground (km/hr)
						}
						else if (GPS_Term==3)
						{ //Mode Indicator
						}
						else if (GPS_Term==4)
						{ //Checksum
						}

					}
					else if (GPS_State==2)
					{//$GPRMC,111636.932,A,2447.0949,N,12100.5223,E,000.0,000.0,030407,,,A*61<CR><LF>
						if (GPS_Term==1)
						{ //UTC Time
							GPSData.UTC_HR 	= ((uint8_t)GPSBuffer[0] - 48)*10 + ((uint8_t)GPSBuffer[1] - 48);
							GPSData.UTC_MIN	= ((uint8_t)GPSBuffer[2] - 48)*10 + ((uint8_t)GPSBuffer[3] - 48);
							GPSData.UTC_SEC	= ((double)GPSBuffer[4] - 48)*10 + ((double)GPSBuffer[5] - 48);
							if (gps_datacount>=9) //7, 8, 9 are digits after decimal place in seconds
							{	GPSData.UTC_SEC	= GPSData.UTC_SEC + ((double)GPSBuffer[7] - 48)/10 + ((double)GPSBuffer[8] - 48)/100 + ((double)GPSBuffer[9] - 48)/1000;}
						}
						else if (GPS_Term==2)
						{ //Status
							if (GPSBuffer[0] == 'A')
							{ //Data is valid
								GPSData.LOCK_STATUS = 1;
							}
							else
							{ //Dta is not valid
								GPSData.LOCK_STATUS = 0;
							}
						}
						else if (GPS_Term==3)
						{  //Latitude
	//						GPSData.LAT_DEG = ((uint8_t)GPSBuffer[0] - 48)*100 + ((uint8_t)GPSBuffer[1] - 48)*10 + ((uint8_t)GPSBuffer[2] - 48);
	//						if (datacount>=3)
	//						{	GPSData.LAT_MIN = ((double)GPSBuffer[3] - 48)*10 + ((double)GPSBuffer[4] - 48);}
	//						else
	//						{	GPSData.LAT_MIN = 0;}
	//
	//						if (datacount>=9) //5, 6, 7, 8 are digits after decimal place in minutes
	//						{
	//							GPSData.LAT_MIN = GPSData.LAT_MIN + ((double)GPSBuffer[6] - 48)/10 + ((double)GPSBuffer[7] - 48)/100+ ((double)GPSBuffer[8] - 48)/1000+ ((double)GPSBuffer[9] - 48)/10000;
	//						}
						}
						else if (GPS_Term==4)
						{ //Latitude N or S
	//						if(GPSBuffer[0] == 'N')
	//						{	GPSData.LAT_NORTH = TRUE;}
	//						else
	//						{	GPSData.LAT_NORTH = FALSE;}
						}
						else if (GPS_Term==5)
						{ //Longitude
	//						GPSData.LONG_DEG = ((uint8_t)GPSBuffer[0] - 48)*100 + ((uint8_t)GPSBuffer[1] - 48)*10 + ((uint8_t)GPSBuffer[2] - 48);
	//						if (datacount>=4)
	//						{	GPSData.LONG_MIN = ((double)GPSBuffer[3] - 48)*10 + ((double)GPSBuffer[4] - 48);}
	//						else
	//						{	GPSData.LONG_MIN = 0;}
	//
	//						if (datacount>=9) //6, 7, 8, 9 are digits after decimal place in minutes
	//						{
	//							GPSData.LONG_MIN = GPSData.LONG_MIN + ((double)GPSBuffer[6] - 48)/10 + ((double)GPSBuffer[7] - 48)/100+ ((double)GPSBuffer[8] - 48)/1000+ ((double)GPSBuffer[9] - 48)/10000;
	//						}

						}
						else if (GPS_Term==6)
						{ //Longitude E/W
	//						if(GPSBuffer[0] == 'E')
	//						{
	//							GPSData.LONG_EAST = TRUE;
	//						}
	//						else
	//						{
	//							GPSData.LONG_EAST = FALSE;
	//						}
						}
						else if (GPS_Term==7)
						{ //Speed over ground
							}
						else if (GPS_Term==8)
						{ //Course over ground
							}
						else if (GPS_Term==9)
						{ //UTC Date
							GPSData.UTC_DAY		= (GPSBuffer[0] - 48)*10 + (GPSBuffer[1] - 48);
							GPSData.UTC_MONTH	= (GPSBuffer[2] - 48)*10 + (GPSBuffer[3] - 48);
							GPSData.UTC_YEAR	= 2000+(GPSBuffer[4] - 48)*10 + (GPSBuffer[5] - 48);

						}
						else if (GPS_Term==10)
						{ //Mode Indicator
						}
						else if (GPS_Term==11)
						{ //Checksum
						}

					}
					else if (GPS_State==3)
					{//$GPGSV,3,3,12,14,39,330,42,01,06,299,38,31,30,256,44,32,36,320,47*7B<CR><LF>
						if (GPS_Term==1)
						{ //Number of Messages

						}
						else if (GPS_Term==2)
						{ //Sequence Number
						}
						else if (GPS_Term==3)
						{ //Satellites in view
							GPSData.SATS = ((uint8_t)GPSBuffer[0] - 48)*10 + ((uint8_t)GPSBuffer[1] - 48);
						}
						else if (GPS_Term==4)
						{ //Satellite ID
						}
						else if (GPS_Term==5)
						{ //Satellite Elevation (deg)
						}
						else if (GPS_Term==6)
						{ //Satellite azimuth (deg)
						}
						else if (GPS_Term==7)
						{ //SNR
						}
						else if (GPS_Term==8)
						{ //Checksum
						}

					}
					else if (GPS_State==4)
					{//$GPGSA,A,3,05,12,21,22,30,09,18,06,14,01,31,,1.2,0.8,0.9*36<CR><LF>

						if (GPS_Term==1)
						{ //Mode

						}
						else if (GPS_Term==2)
						{ //Fix Type
						}
						else if (GPS_Term==3)
						{ //Satellite IDs

						}
						else if (GPS_Term==4)
						{ //PDOP
						}
						else if (GPS_Term==5)
						{ //Horizontal dilution of precision
							//GPSData.HDOP = (GPSBuffer[0] - 48) + (GPSBuffer[1] - 48)/10;
						}
						else if (GPS_Term==6)
						{ //VDOP
						}
						else if (GPS_Term==7)
						{ //Checksum
						}


					}
					else if (GPS_State==5)//$GPGLL
					{//$GPGLL,2447.0944,N,12100.5213,E,112609.932,A,A*57<CR><LF>

						if (GPS_Term==1)
						{  //Latitude
							GPSData.LAT_DEG = ((uint8_t)GPSBuffer[0] - 48)*10 + ((uint8_t)GPSBuffer[1] - 48);
							if (gps_datacount>=3)
							{	GPSData.LAT_MIN = ((double)GPSBuffer[2] - 48)*10 + ((double)GPSBuffer[3] - 48);}
							else
							{	GPSData.LAT_MIN = 0;}

							if (gps_datacount>=8) //5, 6, 7, 8 are digits after decimal place in minutes
							{
								GPSData.LAT_MIN = GPSData.LAT_MIN + ((double)GPSBuffer[5] - 48)/10 + ((double)GPSBuffer[6] - 48)/100+ ((double)GPSBuffer[7] - 48)/1000+ ((double)GPSBuffer[8] - 48)/10000;
							}
						}
						else if (GPS_Term==2)
						{ //Latitude N or S
							if(GPSBuffer[0] == 'N')
							{	GPSData.LAT_NORTH = TRUE;}
							else
							{	GPSData.LAT_NORTH = FALSE;}
						}
						else if (GPS_Term==3)
						{ //Longitude
							GPSData.LONG_DEG = ((uint8_t)GPSBuffer[0] - 48)*100 + ((uint8_t)GPSBuffer[1] - 48)*10 + ((uint8_t)GPSBuffer[2] - 48);
							if (gps_datacount>=4)
							{	GPSData.LONG_MIN = ((double)GPSBuffer[3] - 48)*10 + ((double)GPSBuffer[4] - 48);}
							else
							{	GPSData.LONG_MIN = 0;}

							if (gps_datacount>=9) //6, 7, 8, 9 are digits after decimal place in minutes
							{
								GPSData.LONG_MIN = GPSData.LONG_MIN + ((double)GPSBuffer[6] - 48)/10 + ((double)GPSBuffer[7] - 48)/100+ ((double)GPSBuffer[8] - 48)/1000+ ((double)GPSBuffer[9] - 48)/10000;
							}

						}
						else if (GPS_Term==4)
						{ //Longitude E/W
							if(GPSBuffer[0] == 'E')
							{
								GPSData.LONG_EAST = TRUE;
							}
							else
							{
								GPSData.LONG_EAST = FALSE;
							}
						}
						else if (GPS_Term==5)
						{ //UTC Time
							GPSData.UTC_HR 	= ((uint8_t)GPSBuffer[0] - 48)*10 + ((uint8_t)GPSBuffer[1] - 48);
							GPSData.UTC_MIN	= ((uint8_t)GPSBuffer[2] - 48)*10 + ((uint8_t)GPSBuffer[3] - 48);
							GPSData.UTC_SEC	= ((double)GPSBuffer[4] - 48)*10 + ((double)GPSBuffer[5] - 48);
							if (gps_datacount>=9) //7, 8, 9 are digits after decimal place in seconds
							{	GPSData.UTC_SEC	= GPSData.UTC_SEC + ((double)GPSBuffer[7] - 48)/10 + ((double)GPSBuffer[8] - 48)/100 + ((double)GPSBuffer[9] - 48)/1000;}

						}
						else if (GPS_Term==6)
						{ //status
							if (GPSBuffer[0] == 'A')
							{ //Data is valid
								GPSData.LOCK_STATUS = 1;
							}
							else
							{ //Data is not valid
								GPSData.LOCK_STATUS = 0;
							}
						}
						else if (GPS_Term==7)
						{ //Mode Indicator
						}
						else if (GPS_Term==8)
						{
						}
						else if (GPS_Term==9)
						{
						}

					}
					else if (GPS_State==6)//GPGGA
					{ //$GPGGA,111636.932,2447.0949,N,12100.5223,E,1,11,0.8,118.2,M,,,,0000*02<CR><LF>

						if (GPS_Term==1)
						{ //UTC Time
							GPSData.UTC_HR 	= ((uint8_t)GPSBuffer[0] - 48)*10 + ((uint8_t)GPSBuffer[1] - 48);
							GPSData.UTC_MIN	= ((uint8_t)GPSBuffer[2] - 48)*10 + ((uint8_t)GPSBuffer[3] - 48);
							GPSData.UTC_SEC	= ((double)GPSBuffer[4] - 48)*10 + ((double)GPSBuffer[5] - 48);
							if (gps_datacount>=9) //7, 8, 9 are digits after decimal place in seconds
							{	GPSData.UTC_SEC	= GPSData.UTC_SEC + ((double)GPSBuffer[7] - 48)/10 + ((double)GPSBuffer[8] - 48)/100 + ((double)GPSBuffer[9] - 48)/1000;}
						}
						else if (GPS_Term==2)
						{ //Latitude
							GPSData.LAT_DEG = ((uint8_t)GPSBuffer[0] - 48)*10 + ((uint8_t)GPSBuffer[1] - 48);
							if (gps_datacount>=3)
							{	GPSData.LAT_MIN = ((double)GPSBuffer[2] - 48)*10 + ((double)GPSBuffer[3] - 48);}
							else
							{	GPSData.LAT_MIN = 0;}

							if (gps_datacount>=8) //5, 6, 7, 8 are digits after decimal place in minutes
							{
								GPSData.LAT_MIN = GPSData.LAT_MIN + ((double)GPSBuffer[5] - 48)/10 + ((double)GPSBuffer[6] - 48)/100+ ((double)GPSBuffer[7] - 48)/1000+ ((double)GPSBuffer[8] - 48)/10000;
							}
						}
						else if (GPS_Term==3)
						{ //Latitude N or S
							if(GPSBuffer[0] == 'N')
							{	GPSData.LAT_NORTH = TRUE;}
							else
							{	GPSData.LAT_NORTH = FALSE;}

						}
						else if (GPS_Term==4)
						{ //Longitude
							GPSData.LONG_DEG = ((uint8_t)GPSBuffer[0] - 48)*100 + ((uint8_t)GPSBuffer[1] - 48)*10 + ((uint8_t)GPSBuffer[2] - 48);
							if (gps_datacount>=4)
							{	GPSData.LONG_MIN = ((double)GPSBuffer[3] - 48)*10 + ((double)GPSBuffer[4] - 48);}
							else
							{	GPSData.LONG_MIN = 0;}

							if (gps_datacount>=9) //6, 7, 8, 9 are digits after decimal place in minutes
							{
								GPSData.LONG_MIN = GPSData.LONG_MIN + ((double)GPSBuffer[6] - 48)/10 + ((double)GPSBuffer[7] - 48)/100+ ((double)GPSBuffer[8] - 48)/1000+ ((double)GPSBuffer[9] - 48)/10000;
							}
						}
						else if (GPS_Term==5)
						{ //Longitude E/W
							if(GPSBuffer[0] == 'E')
							{
								GPSData.LONG_EAST = TRUE;
							}
							else
							{
								GPSData.LONG_EAST = FALSE;
							}
						}
						else if (GPS_Term==6)
						{ //GPS quality
							GPSData.LOCK_STATUS = GPSBuffer[0] - 48;
							if (GPSData.LOCK_STATUS>=8)
							{//Data is simulated or not valid
								GPSData.LOCK_STATUS = 0;
							}
						}
						else if (GPS_Term==7)
						{ //Number of sattelites
							GPSData.SATS = ((uint8_t)GPSBuffer[0] - 48)*10 + ((uint8_t)GPSBuffer[1] - 48);
						}
						else if (GPS_Term==8)
						{ //Horizontal dilution of precision
								//GPSData.HDOP = (GPSBuffer[0] - 48) + (GPSBuffer[1] - 48)/10;
						}
						else if (GPS_Term==9)
						{ //Altitude
							if(GPSBuffer[0] == '-')
							{
								GPSData.ALT = -1;
							}
							else
							{
								GPSData.ALT = GPSBuffer[0] - 48;
							}

							for(j=1;j<gps_datacount;j++)
							{
								if(GPSBuffer[j] == '.')
								{
									break;
								}
								GPSData.ALT = ((double)GPSData.ALT*10) + ((double)GPSBuffer[j] - 48);
							}



	//						printf("\r\nTime: %u:%u:%f",GPSData.UTC_HR, GPSData.UTC_MIN, GPSData.UTC_SEC);
	//						printf(", Latitude: %udeg %fmin",GPSData.LAT_DEG, GPSData.LAT_MIN);
	//						if(GPSData.LAT_NORTH == TRUE)
	//						{
	//							printf(" N");
	//						}
	//						else
	//						{
	//							printf(" S");
	//						}
	//						printf(", Longitude: %udeg %fmin",GPSData, GPSData.LONG_DEG, GPSData.LONG_MIN);
	//						if(GPSData.LONG_EAST == TRUE)
	//						{
	//							printf(" E");
	//						}
	//						else
	//						{
	//							printf(" W");
	//						}
	//						printf(", Satellites: %u", GPSData.SATS);
	//						printf(", Altitude: %f\r\n", GPSData.ALT);

						}



					}

				}

				gps_datacount=0;
			}
			else //GPS data is in the middle of a term
			{
				gps_datacount++;
				if (gps_datacount>70)
				{
					gps_datacount=0;//prevent datacount from exceeding maximum GPSBuffer index in the event of an input error
				}
			}




}

