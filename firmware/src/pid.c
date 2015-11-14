/*
 * pid.c
 *
 *  Created on: Oct 29, 2015
 *      Author: pekka
 *      Based on F103ServoDrive project by Mihai
 */

#include <stm32f10x_gpio.h>
#include "pid.h"
#include "pwm.h"
#include "hall.h"
#include "encoder.h"
#include "input.h"

void initPid()
{

	pid_requested_position=encoder_count;
	pid_max_pos_error=1000; //quarter turn
	pid_integrated_error = 0;
	pid_prev_position_error =0;


}
void updatePid()
{
	int32_t position_error;
	uint32_t abs_position_error;
	static uint8_t prevdir;
	if (!motor_running)
	{
		return;
		pid_integrated_error=0;


	}
	//PID_Frequency_Pulse++;

	position_error = encoder_count - pid_requested_position;
	abs_position_error = abs(position_error);

	if (abs_position_error > pid_max_pos_error)
	{
	      pwm_motorStop();
	      ERROR_LED_ON;
	      return;

	}

	int32_t output = position_error * s.pid_Kp;

	pid_integrated_error += position_error * s.pid_Ki;
	if (pid_integrated_error > 2000)
		pid_integrated_error = 2000;
	if (pid_integrated_error < -2000)
		pid_integrated_error = 2000;

	output += pid_integrated_error;

	output += (position_error - pid_prev_position_error) * s.pid_Kd;
	pid_prev_position_error = position_error;

	//Output /= 10; // to be decided...

    //limit output power
	if (output > MAX_DUTY)
		output = MAX_DUTY;
	if(output>0)
	{
		dir=0;

	}
	else
	{
		dir=1;

	}

	if(dir!=prevdir)
	{
		pwm_InitialBLDCCommutation();
	}
	prevdir=dir;
	pwm_setDutyCycle(abs(output));


}
