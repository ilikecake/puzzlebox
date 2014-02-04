
#include "main.h"
#include "GPIO_Interrupt.h"

void GPIOInt_Init()
{
	//Setup EINT3 for interrupting on GPIO pins

	//setup input pin from accelerometer
  	Chip_IOCON_PinMux(LPC_IOCON, 0, 16, IOCON_MODE_INACT, IOCON_FUNC0);//set pin to high impedence
	Chip_GPIO_WriteDirBit(LPC_GPIO, 0, 16, false); //Set pin P0.16 to input (accelerometer INT1)

	//Chip_GPIO_WriteIntBit(LPC_GPIOINT_IO0IntEnR,0,16,1);//set EINT3 to trigger on P0.16 rising edge.  This comed from the accelerometer

	// Enabled External Interrupt 3
	//NVIC_EnableIRQ(EINT3_IRQn);
}



void Chip_GPIO_WriteIntBit(uint32_t Register, uint8_t Port, uint32_t bit, uint8_t setting)
{
	uint32_t GPIOINT_Register;

	GPIOINT_Register = LPC_GPIOINT_BASE | Register;

	if (setting==0)
	{
		GPIOINT_Register &= (uint32_t)1<<bit;
	}
	else
	{
		GPIOINT_Register |= (uint32_t)1<<bit;
	}

}



void EINT3_IRQHandler(void)
{
	uint8_t returnCode;
	uint8_t regvalue;

	Board_LED_Set(2, 1);//turn on LED 2

	printf("EINT3 interrupt");

	// Clear interrupt
	Chip_GPIO_WriteIntBit(LPC_GPIOINT_IO0IntClr,0,16,1);//clear the interrupt for P0.16

	//reset lsm303 interrupt latch
	returnCode = LSM303ReadReg(LSM303_ACCELEROMETER_ADDRESS, LSM303_ACCELEROMETER_INT1_SOURCE, &regvalue);



}
