/*
 	bldc-drive Cheap and simple brushless DC motor driver designed for CNC applications using STM32 microcontroller.
	Copyright (C) 2015 Pekka Roivainen

	Pid loop is based on F103ServoDrive project of Mihai.
	http://www.cnczone.com/forums/open-source-controller-boards/283428-cnc.html

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

#include <stm32f10x_gpio.h>
#include <stdlib.h>
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

	//TIM3 used for pid loop timing.
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 72; //1MHz counter
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 250; //4kHz update interval
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}
//This is called from TIM3 update interrupt. defined in input.c
void updatePid()
{
	int32_t position_error;
	uint32_t abs_position_error;
	static uint8_t prevdir;
	if (!motor_running)
	{
		pid_integrated_error=0;
		return;



	}
	getEncoderCount();


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

	output /= 100; //provide larger dynamic range for pid. (without this, having pid_Ki = 1 was enough for oscillation.

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
		prevdir=dir;
	}

	pwm_setDutyCycle(abs(output));


}
