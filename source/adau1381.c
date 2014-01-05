//  ADAU1381 Driver
//  9/8/2011 - PDS

//#include "main.h"
#include "adau1381.h"
#include "hardware.h"
#include "LPC17xx.h"

#include "lpc17xx_i2c.h"
#include "lpc17xx_gpio.h"

Bool ADAU1381Init( void )
{
	uint8_t PLLSetup[6];
	
	PLLSetup[0] = 0x00;		//Denominator MSB
	PLLSetup[1] = 0x00;		//Denominator LSB
	PLLSetup[2] = 0x00;		//Numerator MSB
	PLLSetup[3] = 0x00;		//Numerator LSB
	PLLSetup[4] = 0x02;		//R=4, X=0, integer PLL
	PLLSetup[5] = 0x00;		//Enable PLL

	//Setup GPIO line for oscilator enable
	GPIO_SetDir(2, 0x00000001, 1);		//Oscilator enable is P2.0
	GPIO_SetDir(1, (1<<8), 1);			//Codec powerdown is P1.8
	
	ADAU1381OscEnable(1);				//Enable oscilator
	ADAU1381PowerDown(1);				//Power down ADAU1381
	Delay(5);							//Wait 5ms
	//ADAU1381PowerDown(0);				//Power up ADAU1381
	Delay(20);							//Wait 20ms for POR and initalization
	
	//Setup on-chip register for basic functionality
	if(ADAU1381SetPLL(PLLSetup))											//Setup PLL
	{
		if(ADAU1381SetProperty(ADAU1381_CLOCK_CONTROL, 0x01))				//Setup master clock for 12.288MHz input
		{
			Delay(6);														//Wait 6ms clock to stabalize
			if(ADAU1381SetProperty(ADAU1381_DIGITAL_POWERDOWN_0, 0x41))		//Powerup memory controller and sound engine
			{
				ADAU1381SetProperty(ADAU1381_SERIAL_PORT_CONTROL_0, 0x01);		//Setup serial port for I2S slave mode
				ADAU1381SetProperty(ADAU1381_SERIAL_PORT_CONTROL_1, 0x20);		//Setup I2S for 32 clocks-per-frame, big-endian
				ADAU1381SetProperty(ADAU1381_CONVERTER_CONTROL_0, 0x00);		//Sample rate is 48kHz
				
				ADAU1381SetProperty(ADAU1381_PLAYBACK_MIXER_LEFT, 0x20);		//Unmute left DAC
				ADAU1381SetProperty(ADAU1381_PLAYBACK_MIXER_RIGHT, 0x20);		//Unmute right DAC
				
				ADAU1381SetProperty(ADAU1381_LEFT_LINE_OUT_MUTE, 0x02);			//Unmute left line out
				ADAU1381SetProperty(ADAU1381_RIGHT_LINE_OUT_MUTE, 0x02);		//Unmute right line out
				
				ADAU1381SetProperty(ADAU1381_PLAYBACK_POWER_MANAGMENT, 0x03);	//Enable left and right playback backends
				ADAU1381SetProperty(ADAU1381_DAC_CONTROL, 0x03);				//Enable both DACs
				
				ADAU1381SetProperty(ADAU1381_DIGITAL_POWERDOWN_0, 0x77);		//Power up playback digital systems
				ADAU1381SetProperty(ADAU1381_DIGITAL_POWERDOWN_1, 0x0F);		//Power up DAC engine
				
				ADAU1381SetProperty(ADAU1381_SOUND_ENGINE_FRAME_RATE, 0x01);	//Sound engine frame rate is 48kHz
				ADAU1381SetProperty(ADAU1381_SERIAL_INPUT_ROUTE, 0x01);			//Bypass the sound engine
				ADAU1381SetProperty(ADAU1381_SOUND_ENGINE_RUN, 0x00);			//Leave sound engine in standby
				ADAU1381SetProperty(ADAU1381_SERIAL_PORT_SAMPLE_RATE, 0x00);	//Serial port sample rate is 48kHz
				return TRUE;
			}
		}
	}
	return FALSE;
}

void ADAU1381OscEnable(uint8_t inval)
{
	if(inval == 1)			//Enable
	{
		GPIO_SetValue(2, 0x00000001);
	}
	else if (inval == 0)	//Disable
	{
		GPIO_ClearValue(2, 0x00000001);
	}
	return;
}

//1 to shutdown
void ADAU1381PowerDown(uint8_t SD)
{
	if(SD == 1)				//Turn off
	{
		GPIO_ClearValue(1, (1<<8));
	}
	else if(SD == 0)		//Turn on
	{
		GPIO_SetValue(1, (1<<8));
	}

}

int16_t ADAU1381GetProperty(int16_t property, uint8_t *inBuffer)
{
	I2C_M_SETUP_Type ADAU1381_I2C;
	uint8_t ADAU1381WriteBuffer[2];
	uint8_t ADAU1381ReadBuffer;
	
	if(property != ADAU1381_PLL_CONTROL)	//This is a 48bit register, use ADAU1381GetPLL to access.
	{
		ADAU1381_I2C.sl_addr7bit = ADAU1381_ADDRESS;

		ADAU1381WriteBuffer[0] = ((property>>8) & 0xFF);
		ADAU1381WriteBuffer[1] = (property & 0xFF);
		
		ADAU1381_I2C.tx_data = ADAU1381WriteBuffer;
		ADAU1381_I2C.tx_length = 2;
		ADAU1381_I2C.rx_data = &ADAU1381ReadBuffer;
		ADAU1381_I2C.rx_length = 1;
		ADAU1381_I2C.retransmissions_max = 3;

		if (I2C_MasterTransferData(I2CDEV, &ADAU1381_I2C, I2C_TRANSFER_POLLING) == SUCCESS)
		{
			*inBuffer = ADAU1381ReadBuffer;
			return TRUE;
		}
	}
	return FALSE;
}

Bool ADAU1381SetProperty(int16_t property, uint8_t value)
{
	I2C_M_SETUP_Type ADAU1381_I2C;
	uint8_t ADAU1381WriteBuffer[3];
	uint8_t ADAU1381ReadBuffer;
	
	if(property != ADAU1381_PLL_CONTROL)	//This is a 48bit register, use ADAU1381SetPLL to access.
	{
		ADAU1381_I2C.sl_addr7bit = ADAU1381_ADDRESS;
		ADAU1381WriteBuffer[0] = (property>>8) & 0xFF;
		ADAU1381WriteBuffer[1] = (property & 0xFF);
		ADAU1381WriteBuffer[2] = value;
		
		ADAU1381_I2C.tx_data = ADAU1381WriteBuffer;
		ADAU1381_I2C.tx_length = 3;
		ADAU1381_I2C.rx_data = &ADAU1381ReadBuffer;
		ADAU1381_I2C.rx_length = 0;
		ADAU1381_I2C.retransmissions_max = 3;

		if (I2C_MasterTransferData(I2CDEV, &ADAU1381_I2C, I2C_TRANSFER_POLLING) == SUCCESS)
		{
			return (TRUE);
		}
	}
	return (FALSE);
}

Bool ADAU1381SetPLL(uint8_t *inBuffer)
{
	I2C_M_SETUP_Type ADAU1381_I2C;
	uint8_t ADAU1381WriteBuffer[8];
	uint8_t ADAU1381ReadBuffer;

	ADAU1381_I2C.sl_addr7bit = ADAU1381_ADDRESS;
	ADAU1381WriteBuffer[0] = (ADAU1381_PLL_CONTROL >> 8) & 0xFF;
	ADAU1381WriteBuffer[1] = (ADAU1381_PLL_CONTROL & 0xFF);
	ADAU1381WriteBuffer[2] = inBuffer[0];
	ADAU1381WriteBuffer[3] = inBuffer[1];
	ADAU1381WriteBuffer[4] = inBuffer[2];
	ADAU1381WriteBuffer[5] = inBuffer[3];
	ADAU1381WriteBuffer[6] = inBuffer[4];
	ADAU1381WriteBuffer[7] = inBuffer[5];
		
	ADAU1381_I2C.tx_data = ADAU1381WriteBuffer;
	ADAU1381_I2C.tx_length = 8;
	ADAU1381_I2C.rx_data = &ADAU1381ReadBuffer;
	ADAU1381_I2C.rx_length = 0;
	ADAU1381_I2C.retransmissions_max = 3;

	if (I2C_MasterTransferData(I2CDEV, &ADAU1381_I2C, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		return (TRUE);
	}
	return (FALSE);
}

Bool ADAU1381GetPLL(uint8_t *inBuffer)
{
	I2C_M_SETUP_Type ADAU1381_I2C;
	uint8_t ADAU1381WriteBuffer[2];
	uint8_t ADAU1381ReadBuffer[6];

	ADAU1381_I2C.sl_addr7bit = ADAU1381_ADDRESS;
	ADAU1381WriteBuffer[0] = (ADAU1381_PLL_CONTROL >> 8) & 0xFF;
	ADAU1381WriteBuffer[1] = (ADAU1381_PLL_CONTROL & 0xFF);
		
	ADAU1381_I2C.tx_data = ADAU1381WriteBuffer;
	ADAU1381_I2C.tx_length = 2;
	ADAU1381_I2C.rx_data = ADAU1381ReadBuffer;
	ADAU1381_I2C.rx_length = 6;
	ADAU1381_I2C.retransmissions_max = 3;

	if (I2C_MasterTransferData(I2CDEV, &ADAU1381_I2C, I2C_TRANSFER_POLLING) == SUCCESS)
	{
		inBuffer[0] = ADAU1381ReadBuffer[0];
		inBuffer[1] = ADAU1381ReadBuffer[1];
		inBuffer[2] = ADAU1381ReadBuffer[2];
		inBuffer[3] = ADAU1381ReadBuffer[3];
		inBuffer[4] = ADAU1381ReadBuffer[4];
		inBuffer[5] = ADAU1381ReadBuffer[5];
		return (TRUE);
	}
	return (FALSE);
}