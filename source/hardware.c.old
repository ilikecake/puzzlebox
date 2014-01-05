//Hardware functions for puzzle box mainboard
//9/3/2011 - PDS

#include "main.h"

//uint16_t A440[110] = {0,1727,3448,5158,6851,8520,10162,11770,13339,14864,16339,17760,19123,20422,21653,22812,23896,24900,25822,26659,27406,28063,28627,29096,29469,29743,29919,29996,29974,29852,29631,29311,28895,28383,27776,27078,26289,25414,24454,23413,22294,21102,19839,18511,17121,15675,14177,12631,11044,9420,7765,6084,4382,2667,942,-785,-2510,-4227,-5930,-7613,-9271,-10898,-12488,-14038,-15541,-16992,-18387,-19721,-20990,-22189,-23314,-24363,-25330,-26213,-27010,-27716,-28331,-28852,-29278,-29606,-29836,-29967,-29998,-29931,-29763,-29498,-29134,-28674,-28118,-27470,-26730,-25902,-24988,-23991,-22914,-21761,-20536,-19243,-17887,-16471,-15000,-13480,-11914,-10310,-8671,-7003,-5313,-3604,-1884};

int externalStatus = 0;
void SysTick_Handler (void);
volatile unsigned long SysTickCnt;

//Note: clocks and PLL are allready set up.
void HWInit (void)
{
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	I2S_MODEConf_Type I2S_ClkConfig;
	I2S_CFG_Type I2S_ConfigStruct;
	PINSEL_CFG_Type PinCfg;
	RTC_TIME_Type TheCurrentTime;

	//Setup timer to generate interrupts every 1ms
	SysTick_Config(SystemCoreClock/1000 - 1);

	//Setup LEDs
	GPIO_SetDir(1, 0x00000013, 1);		//Set LED pins as output
	GPIO_SetValue(1, 0x00000013);		//Set LED pins high (LEDs off)
	
	//-------------------------------------------------------------
	//  I2C Initialization
	//-------------------------------------------------------------
	
	//Setup Pins for I2C
	PinCfg.OpenDrain 	= PINSEL_PINMODE_NORMAL;	//Not open drain
	PinCfg.Pinmode 		= PINSEL_PINMODE_TRISTATE;	//
	PinCfg.Funcnum 		= PINSEL_FUNC_3;
	PinCfg.Pinnum 		= 0;
	PinCfg.Portnum 		= 0;
	PINSEL_ConfigPin(&PinCfg);
	
	PinCfg.Pinnum 		= 1;
	PINSEL_ConfigPin(&PinCfg);

	//Initialize I2C
	I2C_Init(I2CDEV, 100000);

	//Enable I2C operation
	I2C_Cmd(I2CDEV, ENABLE);
	
	//-------------------------------------------------------------
	//  I2S Initialization
	//-------------------------------------------------------------
	
	//Setup pins for I2S
	//	- P0.6 as I2SRX_SDA
	//	- P0.7 as I2STX_CLK
	//	- P0.8 as I2STX_WS
	//	- P0.9 as I2STX_SDA
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
	PinCfg.Pinnum = 6;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);

	I2S_Init(LPC_I2S);

	//Setup for I2S: RX is similar with TX
	// 		- wordwidth: 16 bits
	// 		- stereo mode
	// 		- master mode for I2S_TX and slave for I2S_RX
	// 		- ws_halfperiod is 31
	// 		- not use mute mode
	// 		- use reset and stop mode
	// 		- select the fractional rate divider clock output as the source,
	// 		- disable 4-pin mode
	// 		- MCLK ouput is disable
	// 		- Frequency = 44.1 kHz (x=8,y=51 - automatic setting)
	// Because we use mode I2STXMODE[3:0]= 0000, I2SDAO[5]=0 and
	// I2SRX[3:0]=0000, I2SDAI[5] = 1. So we have I2SRX_CLK = I2STX_CLK
	// --> I2SRXBITRATE = 1 (not divide TXCLK to produce RXCLK)
	//

	//Audio Config
	I2S_ConfigStruct.wordwidth = I2S_WORDWIDTH_16;
	I2S_ConfigStruct.mono = I2S_STEREO;
	I2S_ConfigStruct.stop = I2S_STOP_ENABLE;
	I2S_ConfigStruct.reset = I2S_RESET_ENABLE;
	I2S_ConfigStruct.ws_sel = I2S_SLAVE_MODE;
	I2S_ConfigStruct.mute = I2S_MUTE_DISABLE;
	I2S_Config(LPC_I2S,I2S_TX_MODE,&I2S_ConfigStruct);
	//I2S_Config(LPC_I2S,I2S_RX_MODE,&I2S_ConfigStruct);

	//Clock Mode Config
	I2S_ClkConfig.clksel = I2S_CLKSEL_FRDCLK;
	I2S_ClkConfig.fpin = I2S_4PIN_DISABLE;
	I2S_ClkConfig.mcena = I2S_MCLK_DISABLE;
	I2S_ModeConfig(LPC_I2S,&I2S_ClkConfig,I2S_TX_MODE);
	//I2S_ModeConfig(LPC_I2S,&I2S_ClkConfig,I2S_RX_MODE);

	//Set up frequency and bit rate
	//I2S_FreqConfig(LPC_I2S, 48000, I2S_TX_MODE);
	//I2S_SetBitRate(LPC_I2S, 0, I2S_RX_MODE);
	
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
	
	PinCfg.Funcnum 		= PINSEL_FUNC_3;
	PinCfg.OpenDrain 	= PINSEL_PINMODE_NORMAL;
	PinCfg.Pinmode 		= PINSEL_PINMODE_PULLUP;
	PinCfg.Portnum 		= 1;
	PinCfg.Pinnum 		= 20;
	PINSEL_ConfigPin(&PinCfg);
	
	PinCfg.Pinnum 		= 23;
	PINSEL_ConfigPin(&PinCfg);
	
	PinCfg.Pinnum 		= 24;
	PINSEL_ConfigPin(&PinCfg);
	
	
	//Setup port 2.0 as GPIO
	PinCfg.OpenDrain 	= PINSEL_PINMODE_NORMAL;	//Not open drain
	PinCfg.Pinmode 		= PINSEL_PINMODE_PULLUP;	//
	PinCfg.Funcnum 		= PINSEL_FUNC_0;
	PinCfg.Pinnum 		= 0;
	PinCfg.Portnum 		= 2;
	//PINSEL_ConfigPin(&PinCfg);
	

	//Setup GPIO Interrupts
	NVIC_SetPriority( EINT3_IRQn, GPIO_INTERRUPT_PRIORITY );
	NVIC_EnableIRQ( EINT3_IRQn );
	
	//Setup Pins for buttons
	PinCfg.OpenDrain 	= PINSEL_PINMODE_NORMAL;	//Not open drain
	PinCfg.Pinmode 		= PINSEL_PINMODE_PULLDOWN;	//
	PinCfg.Funcnum 		= PINSEL_FUNC_0;
	PinCfg.Pinnum 		= 4;
	PinCfg.Portnum 		= 2;
	PINSEL_ConfigPin(&PinCfg);
	
	PinCfg.Pinnum 		= 5;
	PINSEL_ConfigPin(&PinCfg);
	
	PinCfg.Pinnum 		= 6;
	PINSEL_ConfigPin(&PinCfg);
	
	//Enable GPIO interrupts
	//GPIO_IntCmd(0, (1<<17), 1);							//Falling edge, Port 0
	GPIO_IntCmd(2, ((1<<4)|(1<<5)|(1<<6)|(1<<8)), 0);		//Rising edge, Port 2
	
	//Setup timer 2 for general timing
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;	//prescale value in microseconds
	TIM_ConfigStruct.PrescaleValue	= 1;					//prescale value is 1us
	TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &TIM_ConfigStruct);

	//Setup RTC
	TheCurrentTime.SEC = 0;
	TheCurrentTime.MIN = 0;
	TheCurrentTime.HOUR = 12;
	TheCurrentTime.DOM = 23;
	TheCurrentTime.MONTH = 3;
	TheCurrentTime.YEAR = 2012;
	
	RTC_Init(LPC_RTC);
	
	RTC_ResetClockTickCounter(LPC_RTC);
	RTC_Cmd(LPC_RTC, ENABLE);
	RTC_CalibCounterCmd(LPC_RTC, DISABLE);
	RTC_SetFullTime(LPC_RTC, &TheCurrentTime);
	
	LPC_RTC->RTC_AUX = 0x00000010;
		
	return;
}

//Set LED state
//	led - LED number to use (1-3)
//	state - 0 for off
//  	  - 1 for on
//All other values for led and state are ignored.		  
void LED (unsigned char led, unsigned char state)
{
	switch (led)
	{
		case 1:
			if (state == 0)
			{
				GPIO_SetValue(1, 0x00000001);
			}
			else if (state == 1)
			{
				GPIO_ClearValue(1, 0x00000001);
			}
			break;
			
		case 2:
			if (state == 0)
			{
				GPIO_SetValue(1, 0x00000002);
			}
			else if (state == 1)
			{
				GPIO_ClearValue(1, 0x00000002);
			}
			break;
			
		case 3:
			if (state == 0)
			{
				GPIO_SetValue(1, 0x00000010);
			}
			else if (state == 1)
			{
				GPIO_ClearValue(1, 0x00000010);
			}
			break;
	}
	return;
}


void EINT3_IRQHandler (void)
{
	uint8_t temp;
	uint8_t i;
	//int IntStatus;
	
	//Handle interrupts on port 0 pins
	if(GPIO_GetIntStatus(0, 17, 1) == ENABLE)	//P0.17 falling edge
	{
		//Accelerometer data ready
		LED(1,1);
		AT42QT1060ReadByte(AT42QT1060_REG_DETECT_STATUS, &temp);
		for(i=0;i<6;i++)
		{
			if((temp & (1<<i)) == (1<<i))
			{
				VCOM_printf("Button %d\n", i+1);
			}
		}
	}
	
	//Handle interrupts on port 2 pins
	
	//Button 1
	if(GPIO_GetIntStatus(2, 4, 0) == ENABLE)	//P2.4 rising edge
	{
		LED(1,0);
		LED(2,0);
		LED(3,0);
	}
	
	//Button 2
	if(GPIO_GetIntStatus(2, 5, 0) == ENABLE)	//P2.5 rising edge
	{
		LED(1,0);
		LED(2,0);
		LED(3,0);
	}
	
	//Button 3
	if(GPIO_GetIntStatus(2, 6, 0) == ENABLE)	//P2.6 rising edge
	{
		LED(1,0);
		LED(2,0);
		LED(3,0);
	}
	
	//Magnetometer Data Ready
	if(GPIO_GetIntStatus(2, 8, 0) == ENABLE)	//P2.8 rising edge
	{
		LED(1,1);
	}
	
	
	
	/*IntStatus = LPC_GPIOINT->IO2IntStatR;
	switch (IntStatus)
	{
		case (1<<4):		//Button 1
			LED(1,0);
			LED(2,0);
			LED(3,0);
			break;
		
		case (1<<5):		//Button 2
			LED(1,0);
			LED(2,0);
			LED(3,0);
			break;
			
		case (1<<6):		//Button 3
			LED(1,0);
			LED(2,0);
			LED(3,0);
			break;
			
		case (1<<8):		//Magnetometer Data Ready
			/*LSM303ReadData(LSM303_MAGNETOMETER);
			HIDOutputCommand[0] 	= LSM303ReadBuffer[0];
			HIDOutputCommand[1] 	= LSM303ReadBuffer[1];
			HIDOutputCommand[2] 	= LSM303ReadBuffer[2];
			HIDOutputCommand[3] 	= LSM303ReadBuffer[3];
			HIDOutputCommand[4] 	= LSM303ReadBuffer[4];
			HIDOutputCommand[5] 	= LSM303ReadBuffer[5];
			
			LSM303ReadData(LSM303_ACCELEROMETER);
			HIDOutputCommand[6] 	= LSM303ReadBuffer[0];
			HIDOutputCommand[7] 	= LSM303ReadBuffer[1];
			HIDOutputCommand[8] 	= LSM303ReadBuffer[2];
			HIDOutputCommand[9] 	= LSM303ReadBuffer[3];
			HIDOutputCommand[10] 	= LSM303ReadBuffer[4];
			HIDOutputCommand[11] 	= LSM303ReadBuffer[5];
			
			if(externalStatus == 0)
			{
				LED(3,1);
				externalStatus = 1;
			}
			else
			{
				LED(3,0);
				externalStatus = 0;
			}*/
			
			//break;
	//}

	//LED(2,1);



	//Clear interrupts
	GPIO_ClearInt(0, 0xFFFFFFFF);
	GPIO_ClearInt(2, 0xFFFFFFFF);
	return;
}

/*********************************************************************//**
 * @brief		SysTick handler sub-routine (1ms)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void SysTick_Handler (void) {
  SysTickCnt++;
}

/*********************************************************************//**
 * @brief		Delay function
 * @param[in]	tick - number milisecond of delay time
 * @return 		None
 **********************************************************************/
void Delay (unsigned long tick) 
{
  unsigned long systickcnt;

  systickcnt = SysTickCnt;
  while ((SysTickCnt - systickcnt) < tick);
}

Bool AnyKeyPressed(void)
{
	int16_t AnyKey = EOF;
	AnyKey = VCOM_getchar();
	if(AnyKey != EOF)
	{
		return TRUE;
	}
	return FALSE;
}

void StartTimer(void)
{
	TIM_ResetCounter(LPC_TIM2);
	TIM_Cmd(LPC_TIM2,ENABLE);
}

uint32_t StopTimer(void)
{
	TIM_Cmd(LPC_TIM2,DISABLE);
	return(LPC_TIM2->TC);
}


//void I2CStart(void)
//{
//	PINSEL_CFG_Type PinCfg;
//	I2S_MODEConf_Type I2S_ClkConfig;
//	I2S_CFG_Type I2S_ConfigStruct;
//	uint32_t I2SDataToTX;
//	int i;
//	int I2STXFIFO_level;
//
//	i = 0;
//
//
//	/* Pin configuration:
//	 * Assign:	- P0.6 as I2SRX_SDA
//	 * 			- P0.7 as I2STX_CLK
//	 * 			- P0.8 as I2STX_WS
//	 * 			- P0.9 as I2STX_SDA
//	 */
//	PinCfg.Funcnum = 1;
//	PinCfg.OpenDrain = 0;
//	PinCfg.Pinmode = 0;
//	PinCfg.Pinnum = 6;
//	PinCfg.Portnum = 0;
//	PINSEL_ConfigPin(&PinCfg);
//	PinCfg.Pinnum = 7;
//	PINSEL_ConfigPin(&PinCfg);
//	PinCfg.Pinnum = 8;
//	PINSEL_ConfigPin(&PinCfg);
//	PinCfg.Pinnum = 9;
//	PINSEL_ConfigPin(&PinCfg);
//
//	I2S_Init(LPC_I2S);
//
//	//Setup for I2S: RX is similar with TX
//	/* setup:
//	 * 		- wordwidth: 16 bits
//	 * 		- stereo mode
//	 * 		- master mode for I2S_TX and slave for I2S_RX
//	 * 		- ws_halfperiod is 31
//	 * 		- not use mute mode
//	 * 		- use reset and stop mode
//	 * 		- select the fractional rate divider clock output as the source,
//	 * 		- disable 4-pin mode
//	 * 		- MCLK ouput is disable
//	 * 		- Frequency = 44.1 kHz (x=8,y=51 - automatic setting)
//	 * Because we use mode I2STXMODE[3:0]= 0000, I2SDAO[5]=0 and
//	 * I2SRX[3:0]=0000, I2SDAI[5] = 1. So we have I2SRX_CLK = I2STX_CLK
//	 * --> I2SRXBITRATE = 1 (not divide TXCLK to produce RXCLK)
//	 */
//
//	/* Audio Config*/
//	I2S_ConfigStruct.wordwidth = I2S_WORDWIDTH_16;
//	I2S_ConfigStruct.mono = I2S_STEREO;
//	I2S_ConfigStruct.stop = I2S_STOP_ENABLE;
//	I2S_ConfigStruct.reset = I2S_RESET_ENABLE;
//	I2S_ConfigStruct.ws_sel = I2S_SLAVE_MODE;
//	I2S_ConfigStruct.mute = I2S_MUTE_DISABLE;
//	I2S_Config(LPC_I2S,I2S_TX_MODE,&I2S_ConfigStruct);
//
//	//I2S_ConfigStruct.ws_sel = I2S_SLAVE_MODE;
//	//I2S_Config(LPC_I2S,I2S_RX_MODE,&I2S_ConfigStruct);
//
//	/* Clock Mode Config*/
//	I2S_ClkConfig.clksel = I2S_CLKSEL_FRDCLK;
//	I2S_ClkConfig.fpin = I2S_4PIN_DISABLE;
//	I2S_ClkConfig.mcena = I2S_MCLK_DISABLE;
//	I2S_ModeConfig(LPC_I2S,&I2S_ClkConfig,I2S_TX_MODE);
//	//I2S_ModeConfig(LPC_I2S,&I2S_ClkConfig,I2S_RX_MODE);
//
//	/* Set up frequency and bit rate*/
//	I2S_FreqConfig(LPC_I2S, 48000, I2S_TX_MODE);
//	//I2S_SetBitRate(LPC_I2S, 0, I2S_RX_MODE);
//
//	I2S_Start(LPC_I2S);
//	while(1)
//	{
//		I2STXFIFO_level = I2S_GetLevel(LPC_I2S, I2S_TX_MODE);
//		if(I2STXFIFO_level < 6)
//		{
//			I2SDataToTX = 0xAA0000AA;//A440[i];
//			if(i<109)
//			{
//				i++;
//			}
//			else
//			{
//				i = 0;
//			}
//			I2S_Send(LPC_I2S,I2SDataToTX);
//		}
//	}
//}