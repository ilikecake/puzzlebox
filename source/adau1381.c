//  ADAU1381 Driver
//  9/8/2011 - PDS


#include "adau1381.h"
#include "hardware.h"
#include "LPC17xx.h"

#include "lpc17xx_i2c.h"
#include "lpc17xx_gpio.h"

void ADAU1381Init( void )
{
	//Setup GPIO line for oscilator enable
	GPIO_SetDir(2, 0x00000001, 1);		//Oscilator enable is P2.0
	GPIO_SetValue(2, 0x00000001);		//P2.0 is high to enable



	return;
}

void ADAU1381OscEnable(int inval)
{
	if(inval == 1)
	{
		GPIO_SetValue(2, 0x00000001);
	}
	else if (inval == 0)
	{
		GPIO_ClearValue(2, 0x00000001);
	}
	return;
}