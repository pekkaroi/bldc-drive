/*
 * pwm.h
 *
 *  Created on: Oct 26, 2015
 *      Author: pekka
 */

#ifndef PWM_H
#define PWM_H
#include "configuration.h"
#define BLDC_CHOPPER_PERIOD 4000
#define MAX_DUTY 3950 //100% duty not allowed to allow recharge of high side gate drivers
#define BLDC_NOL 7 //Non-OverLapping, number of clock cycles
#define BLDC_DELAY 30 //Commutation delay. 1= no delay, 2000=7ms.


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


