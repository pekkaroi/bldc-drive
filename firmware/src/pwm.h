/*
 	bldc-drive Cheap and simple brushless DC motor driver designed for CNC applications using STM32 microcontroller.
	Copyright (C) 2015 Pekka Roivainen

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef PWM_H
#define PWM_H
#include "configuration.h"
#define BLDC_CHOPPER_PERIOD 4000
#define MAX_DUTY 3950 //100% duty not allowed to allow recharge of high side gate drivers
#define BLDC_NOL 7 //Non-OverLapping, number of clock cycles
#define BLDC_DELAY 100 //Commutation delay. 1= no delay, 2000=7ms.


static const uint8_t commutation_sequence[6] = {1,3,2,6,4,5};//{001,011,010,110,100,101}
static const uint8_t pos_in_sequence[8] = {0,1,3,2,5,6,4,0};
volatile uint8_t errorInCommutation;
extern volatile servoConfig s;

void enableHallCommutateSignal();
void disableHallCommutateSignal();
void initPWM();
void pwm_BLDCMotorPrepareCommutation();
void pwm_Commute(uint8_t comm_pos);
void pwm_InitialBLDCCommutation();
void pwm_setDutyCycle(uint16_t duty);
void pwm_motorStart();
void pwm_motorStop();

#endif


