/*
 * main.h
 *
 *  Created on: Sep 24, 2013
 *      Author: pat
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>

#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "command.h"
#include "AD5666.h"
#include "i2c.h"
#include "TCA9554A.h"
#include "AD7606.h"
#include "GPS.h"
#include "lsm303.h"
#include "GPIO_Interrupt.h"


void UARTInit(void);


#endif /* MAIN_H_ */
