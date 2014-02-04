//  LSM303 Driver
//Magnetometer-Gyro chip
//  9/4/2011 - PDS


#include "main.h"
#include "lsm303.h"
//#include "hardware.h"
//#include "LPC17xx.h"

//#include "lpc17xx_i2c.h"
//#include "lpc17xx_gpio.h"
//#include "lpc17xx_pinsel.h"

void LSM303Init( void )
{

	//Turn on 1.8V regulator
	Chip_GPIO_WriteDirBit(LPC_GPIO, 2, 1, true); //Set Regulator enable is P2.1 to output
	Chip_GPIO_WritePortBit(LPC_GPIO, 2, 1 , true);//P2.1 is high to enable

	//Initalize I2C
	i2c_app_init(LSM303_I2C_PORT, SPEED_100KHZ);
	/* Set default mode to interrupt */
	i2c_set_mode(LSM303_I2C_PORT, 0);

	//Setup Magnetometer
	LSM303WriteReg(LSM303_MAGNETOMETER_ADDRESS, LSM303_MAGNETOMETER_CRA_REG, LSM303_MAGNETOMETER_DR_15HZ);
	LSM303WriteReg(LSM303_MAGNETOMETER_ADDRESS, LSM303_MAGNETOMETER_CRB_REG, LSM303_MAGNETOMETER_GAIN_1_9G);
	LSM303WriteReg(LSM303_MAGNETOMETER_ADDRESS, LSM303_MAGNETOMETER_MR_REG, LSM303_MAGNETOMETER_MR_CONTINUOUS);

	//set Accelerometer power mode to normal and sample frequency to 50Hz
	LSM303WriteReg(LSM303_ACCELEROMETER_ADDRESS, LSM303_ACCELEROMETER_CTRL_REG1, LSM303_ACCELEROMETER_CR1_NORMAL | LSM303_ACCELEROMETER_CR1_ODR_50 | LSM303_ACCELEROMETER_CR1_ALLON);

	//set Accelerometer to block data update until data is asked for, set to +/- 4g
	LSM303WriteReg(LSM303_ACCELEROMETER_ADDRESS, LSM303_ACCELEROMETER_CTRL_REG4, LSM303_ACCELEROMETER_CR4_BDU | LSM303_ACCELEROMETER_CR4_FS_4G);


	//set device to "sleep".  Wake up on an interrupt
	//LSM303WriteReg(LSM303_ACCELEROMETER_ADDRESS, LSM303_ACCELEROMETER_CTRL_REG5, LSM303_ACCELEROMETER_CR5_WAKE_ON);


	//set interrupt pin to go high when an interrupt is triggered, rout the INT1 interrupt to the INT1 pad, make interrupt latch high
	LSM303WriteReg(LSM303_ACCELEROMETER_ADDRESS, LSM303_ACCELEROMETER_CTRL_REG3, LSM303_ACCELEROMETER_CR3_INT_HIGH | LSM303_ACCELEROMETER_CR3_INT1_SRC_INT1 | LSM303_ACCELEROMETER_CR3_LIR1);//


	//set threshold value for interrupt 1
	LSM303WriteReg(LSM303_ACCELEROMETER_ADDRESS, LSM303_ACCELEROMETER_INT1_THS, 2000);

	//set duration of movement required to trip the interrupt
	LSM303WriteReg(LSM303_ACCELEROMETER_ADDRESS, LSM303_ACCELEROMETER_INT1_DURATION, 100);

	//set Accelerometer to low power mode and sample frequency to 10Hz
	//LSM303WriteReg(LSM303_ACCELEROMETER_ADDRESS, LSM303_ACCELEROMETER_CTRL_REG1, LSM303_ACCELEROMETER_CR1_LOW_POWER_10 | LSM303_ACCELEROMETER_CR1_ALLON);

	//set which accel values will trigger an interrupt (this sets it to interrupt on movement in any DOF)
	//LSM303WriteReg(LSM303_ACCELEROMETER_ADDRESS, LSM303_ACCELEROMETER_INT1_CFG, LSM303_ACCELEROMETER_INT_CFG_6DPOS);
	LSM303WriteReg(LSM303_ACCELEROMETER_ADDRESS, LSM303_ACCELEROMETER_INT1_CFG, LSM303_ACCELEROMETER_INT_CFG_OR | LSM303_ACCELEROMETER_INT_CFG_XHIE | LSM303_ACCELEROMETER_INT_CFG_XLIE);



	return;
}




uint8_t LSM303WriteReg(uint8_t DeviceAddress, uint8_t RegToWrite, uint8_t ValueToWrite)
{
	I2C_XFER_T xfer;
	uint8_t TXBuffer[2];
	uint8_t RXBuffer;

	TXBuffer[0] = RegToWrite;
	TXBuffer[1] = ValueToWrite;

	xfer.slaveAddr = DeviceAddress;
	xfer.txBuff = TXBuffer;
	xfer.txSz = 2;
	xfer.rxBuff = &RXBuffer;
	xfer.rxSz = 0;

	return Chip_I2C_MasterTransfer(LSM303_I2C_PORT, &xfer);
}

uint8_t LSM303ReadReg(uint8_t DeviceAddress, uint8_t RegToRead, uint8_t *RegData)
{
	I2C_XFER_T xfer;
	uint8_t TXBuffer;
	uint8_t RXBuffer;
	uint8_t returnCode;

	TXBuffer = RegToRead;

	xfer.slaveAddr = DeviceAddress;
	xfer.txBuff = &TXBuffer;
	xfer.txSz = 1;
	xfer.rxBuff = &RXBuffer;
	xfer.rxSz = 1;

	returnCode = Chip_I2C_MasterTransfer(LSM303_I2C_PORT, &xfer);
	if (returnCode == I2C_STATUS_DONE)
	{
		*RegData = RXBuffer;
	}
	return returnCode;
}

uint8_t LSM303ReadData(uint8_t DeviceAddress)
{
	I2C_XFER_T xfer;
	uint8_t TXBuffer;
	uint8_t RXBuffer[6];
	uint8_t returnCode;

	xfer.slaveAddr = DeviceAddress;
	if(DeviceAddress ==  LSM303_ACCELEROMETER_ADDRESS)
	{
		TXBuffer = LSM303_ACCELEROMETER_OUT_X_L | 0x80;
	}
	else if (DeviceAddress == LSM303_MAGNETOMETER_ADDRESS)
	{
		TXBuffer = LSM303_MAGNETOMETER_OUT_X_H;
	}


	xfer.txBuff = &TXBuffer;
	xfer.txSz = 1;
	xfer.rxBuff = RXBuffer;
	xfer.rxSz = 6;

	returnCode = Chip_I2C_MasterTransfer(LSM303_I2C_PORT, &xfer);
	//printf("rxbuf0:%X rxbuf1:%X rxbuf2:%X rxbuf3:%X rxbuf4:%X rxbuf5:%X\r\n", RXBuffer[0],RXBuffer[1],RXBuffer[2],RXBuffer[3],RXBuffer[4],RXBuffer[5]);

	if (returnCode == I2C_STATUS_DONE)
	{
		if(DeviceAddress ==  LSM303_ACCELEROMETER_ADDRESS)
		{
			LSM303AccelerometerData[0] = (int16_t)(RXBuffer[0] | (RXBuffer[1] << 8));
			LSM303AccelerometerData[1] = (int16_t)(RXBuffer[2] | (RXBuffer[3] << 8));
			LSM303AccelerometerData[2] = (int16_t)(RXBuffer[4] | (RXBuffer[5] << 8));
		}
		else if (DeviceAddress == LSM303_MAGNETOMETER_ADDRESS)
		{
			LSM303MagnetometerData[0] = (int16_t)(RXBuffer[1] | (RXBuffer[0] << 8));
			LSM303MagnetometerData[1] = (int16_t)(RXBuffer[3] | (RXBuffer[2] << 8));
			LSM303MagnetometerData[2] = (int16_t)(RXBuffer[5] | (RXBuffer[4] << 8));
		}
	}
	else
	{
		printf("read error %u\r\n",returnCode);
	}
	return returnCode;
}


void LSM303InterruptTrip()//port 0.16 INT1 from accelerometer
{

	//set Accelerometer power mode to normal and sample frequency to 50Hz
	LSM303WriteReg(LSM303_ACCELEROMETER_ADDRESS, LSM303_ACCELEROMETER_CTRL_REG1, LSM303_ACCELEROMETER_CR1_NORMAL | LSM303_ACCELEROMETER_CR1_ODR_50 | LSM303_ACCELEROMETER_CR1_ALLON);

	//LSM303WriteReg(LSM303_ACCELEROMETER_ADDRESS, LSM303_ACCELEROMETER_INT1_SOURCE, );//read-only.  Tells which interrupt was tripped
}
