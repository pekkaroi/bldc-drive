//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include "diag/Trace.h"
#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_rcc.h>



#include "pwm.h"
#include "hall.h"
#include "adc.h"
#include "input.h"
#include "encoder.h"
#include "pid.h"
#include "usart.h"
#include "configuration.h"
#include "input.h"

volatile uint8_t dir;
volatile servoConfig s;



int
main()
{


	motor_running=0;
	updateCtr=0;
	dir=1;

	FLASH_Unlock();
	getConfig();
	FLASH_Lock();
	initUSART(s.usart_baud);
	printConfiguration();

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
	errorInCommutation=1;
	uint8_t ena;
	//check if ENA is on already at start. If it is, start motor.
#if ENA_POLARITY == 1
		ena = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);
#else
		ena = (~(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5)))&1;
#endif
	if(ena)
	{
		pwm_motorStart();
		ENABLE_LED_ON;
	}
  // Infinite loop
  while (1)
    {
	  getEncoderCount();
	  if(s.commutationMethod == commutationMethod_Encoder)
	  {
		  if(encoder_commutation_pos != encoder_commutation_table[encoder_shaft_pos])
		  {
			  //usart_sendStr("commutation to ");
			  //usart_sendChar(encoder_commutation_table[encoder_shaft_pos]+48);
			  encoder_commutation_pos = encoder_commutation_table[encoder_shaft_pos];
			  pwm_Commute(encoder_commutation_pos);
			//  usart_sendStr("\n\r");
		  }
	  }
	  if(s.inputMethod==inputMethod_stepDir)
		  updatePid();




       // Add your code here.
    }
}







#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
