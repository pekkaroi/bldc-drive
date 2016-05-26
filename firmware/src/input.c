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


#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_exti.h>

#include "pwm.h"
#include "encoder.h"
#include "pid.h"
#include "input.h"
#include "configuration.h"

void initLeds()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	//LED0 == ENABLE Led
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//LED1 == ERROR Led
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//LED2 == POWER Led
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

}
void initStepDirInput()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//PA6 as STEP input
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//PA7 as DIR pin
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//PA5 as ENA pin
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6); //STEP pin
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource5); //ENA pin

	EXTI_InitTypeDef EXTI_initStructure;
	EXTI_initStructure.EXTI_Line = EXTI_Line6;
	EXTI_initStructure.EXTI_Mode = EXTI_Mode_Interrupt;

if(!is_step_inverted)
	EXTI_initStructure.EXTI_Trigger = EXTI_Trigger_Rising;
else
	EXTI_initStructure.EXTI_Trigger = EXTI_Trigger_Falling;


	EXTI_initStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_initStructure);

	EXTI_initStructure.EXTI_Line = EXTI_Line5;
	EXTI_initStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_initStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_initStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_initStructure);

	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvicStructure.NVIC_IRQChannelSubPriority = 2;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
}

void EXTI9_5_IRQHandler()
{
	uint8_t ena,idir;

	if(EXTI_GetITStatus(EXTI_Line5)!= RESET)
	{
		//ENA PIN INTERRUPT
if(!is_ena_inverted)
		ena = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);
else
		ena = (~(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5)))&1;

		if(ena && !motor_running)
		{

			//enable rose. Start
			pid_requested_position = encoder_count; //reset the desired position to current.
			pwm_motorStart();

			ENABLE_LED_ON;
			ERROR_LED_OFF;
		}
		else if(!ena && motor_running)
		{
			//enable fell. Stop.
			pwm_motorStop();
			ENABLE_LED_OFF;
		}
		else if(!ena)
		{
			ENABLE_LED_OFF;
		}
		EXTI_ClearITPendingBit(EXTI_Line5);
	}

	if(EXTI_GetITStatus(EXTI_Line6)!= RESET)
	{
		//STEP PIN INTERRUPT

if(!is_dir_inverted)
		idir = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7);
else
		idir = (~(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7)))&1;

		if(idir)
		{
			pid_requested_position+=s.encoder_counts_per_step;
		}
		else
			pid_requested_position-=s.encoder_counts_per_step;
		EXTI_ClearITPendingBit(EXTI_Line6);
	}

}

void initPWMInput()
{

	GPIO_InitTypeDef GPIO_InitStructure;

	//TIM3 as PWM input
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//PA7 as DIR pin
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//PA5 as ENA pin
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	EXTI_InitTypeDef EXTI_initStructure;
	EXTI_initStructure.EXTI_Line = EXTI_Line5;
	EXTI_initStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_initStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_initStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_initStructure);

	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvicStructure.NVIC_IRQChannelSubPriority = 2;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);


	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);



	TIM_ICInitTypeDef TIM_ICInit;
if(!is_step_inverted)
	TIM_ICInit.TIM_ICPolarity = TIM_ICPolarity_Rising;
else
	TIM_ICInit.TIM_ICPolarity = TIM_ICPolarity_Falling;

	TIM_ICInit.TIM_ICFilter = 5;
	TIM_ICInit.TIM_Channel = TIM_Channel_1;
	TIM_ICInit.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInit.TIM_ICSelection = TIM_ICSelection_DirectTI;

	TIM_PWMIConfig(TIM3, &TIM_ICInit);
	TIM_CCxCmd(TIM3, TIM_Channel_1, ENABLE);
	TIM_CCxCmd(TIM3, TIM_Channel_2, ENABLE);

	TIM_SelectInputTrigger(TIM3,TIM_TS_TI1FP1);
	TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
	TIM_ITConfig(TIM3, TIM_IT_CC1 | TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3,ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void TIM3_IRQHandler(void) {
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
	if(s.inputMethod==inputMethod_stepDir)
	{
	  updatePid();
	  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
	else
	{
		updateCtr++;
		if(updateCtr>50)
		{
			 //pwm_motorStop();
			 //ERROR_LED_ON;

		}

	}

  }
  if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)
  {

	uint16_t tim3_dc = TIM3->CCR2;
	uint16_t tim3_per = TIM3->CCR1;
	static uint8_t prevdir;
	uint16_t DC;
	if(!is_dir_inverted)
		dir = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7);
	else
		dir = (~(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7)))&1;

	if(dir!=prevdir)
	{

		pwm_InitialBLDCCommutation();
	}
	prevdir=dir;
	if(tim3_per>0 && tim3_per>tim3_dc)
	{
	  DC = ((uint32_t)(BLDC_CHOPPER_PERIOD*tim3_dc)/(uint32_t)tim3_per);
	}
	else
	  DC=0;

	if(DC<BLDC_NOL*3) DC=BLDC_NOL*3;
	pwm_setDutyCycle(DC);
	updateCtr=0;
/*	if(motor_running==0)
	{
		motor_running=1;
		//BLDCMotorPrepareCommutation();

	}*/
	TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);

  }
  else if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
	//if PWM signal is lost for some reason, stop motor

	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

  }
}



