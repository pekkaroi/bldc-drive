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


#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_flash.h>



#include "pwm.h"
#include "hall.h"
#include "adc.h"
#include "input.h"
#include "encoder.h"
#include "pid.h"
#include "usart.h"
#include "configuration.h"
#include "input.h"
#include "utils.h"
volatile uint8_t dir;
volatile servoConfig s;




int main() {

	motor_running=0;
	updateCtr=0;
	dir=1;

	FLASH_Unlock();
	getConfig();

	initUSART(s.usart_baud);
	printConfiguration();
	systickInit(1000);
	initPWM();
	initADC();
	if( s.commutationMethod == commutationMethod_HALL)
	{
		initHALL();
	}

	if(s.inputMethod == inputMethod_stepDir)
	{
		initStepDirInput();
	}
	else if (s.inputMethod == inputMethod_pwmVelocity)
	{
		initPWMInput();
	}
	if(s.inputMethod == inputMethod_stepDir || s.commutationMethod == commutationMethod_Encoder)
	{
		initEncoder();
	}

	if(s.inputMethod == inputMethod_stepDir)
	{
		initPid();
	}
	initLeds();
	POWER_LED_ON;
//	errorInCommutation=1;
	uint8_t ena;
	//check if ENA is on already at start. If it is, start motor.
if(!is_ena_inverted)
		ena = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);
else
		ena = (~(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5)))&1;

	if(ena)
	{
		pwm_motorStart();
		ENABLE_LED_ON;
	}
	//two different types of main loops depending on commutation method
	if(s.commutationMethod == commutationMethod_Encoder)
	{
		while (1)
		{

			  getEncoderCount();
			  if(encoder_commutation_pos != encoder_commutation_table[encoder_shaft_pos])
			  {
				  //usart_sendStr("commutation to ");
				  //usart_sendChar(encoder_commutation_table[encoder_shaft_pos]+48);
				  encoder_commutation_pos = encoder_commutation_table[encoder_shaft_pos];
				  pwm_Commute(encoder_commutation_pos);
				//  usart_sendStr("\n\r");
			  }
		}
    }
	else
	{
		while(1)
		{
			if(serial_stream_enabled && DMA_GetFlagStatus(DMA1_FLAG_TC2) == SET)
			{
				//dma transfer is complete
				usart_send_stream();
			}
		}
	}
}


