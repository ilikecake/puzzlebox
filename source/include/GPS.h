//GPS Header File
//10/13/2011 - PDS


#ifndef _GPS_H_
#define _GPS_H_

//Standard type definitions
#include "lpc_types.h"

typedef struct
{
	//UTC time and date data
	uint8_t UTC_YEAR;
	uint8_t UTC_MONTH;
	uint8_t UTC_DAY;
	uint8_t UTC_HR;
	uint8_t UTC_MIN;
	uint8_t UTC_SEC;
	
	//Position data
	uint8_t LAT_DEG;
	uint8_t LAT_MIN;
	Bool LAT_NORTH;
	uint8_t LONG_DEG;
	uint8_t LONG_MIN;
	Bool LONG_EAST;
	int16_t ALT;
	
	//GPS status data
	uint8_t LOCK_STATUS;
	uint8_t SATS;
} GpsDataStruct;

GpsDataStruct GPSData;
volatile Bool GPSDataReady;



//Function prototypes
void GPSInit (void);
void GPSReset (uint8_t val);
Bool GPSGetData(Bool toScreen);
void UART2_IRQHandler(void);

#endif