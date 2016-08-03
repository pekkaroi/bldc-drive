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
#include "hall.h"
#include "pwm.h"

volatile uint16_t lasthallpos;
volatile uint16_t hallpos;


void initHALL()
{
	//Hall sensor is connected to TIM4
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	lasthallpos = ((GPIO_ReadInputData(GPIOB)>>6) & 0x0007);

	hallpos = ((GPIO_ReadInputData(GPIOB)>>6) & 0x0007);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 126;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_SelectHallSensor(TIM4, ENABLE);
	TIM_SelectInputTrigger(TIM4, TIM_TS_TI1F_ED);
	TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Reset);

	TIM_ICInitTypeDef  TIM_ICInitStructure;

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	// listen to T1, the  HallSensorEvent
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_TRC;
	// Div:1, every edge
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;

	// input noise filter (reference manual page 322)
	TIM_ICInitStructure.TIM_ICFilter = 0xF;
	TIM_ICInit(TIM4, &TIM_ICInitStructure);

	TIM_OCInitTypeDef  TIM_OCInitStructure;

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_Pulse = BLDC_DELAY; // 1 is no delay; 2000 = 7ms
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);

	// clear interrupt flag
	TIM_ClearFlag(TIM4, TIM_FLAG_CC2);


	TIM_SelectOutputTrigger(TIM4, TIM_TRGOSource_OC2Ref);

	TIM_ITConfig(TIM4, TIM_IT_CC1 | TIM_IT_CC2, ENABLE);

	TIM_Cmd(TIM4, ENABLE);

	// we use preemption interrupts here,  BLDC Bridge switching and
	// Hall has highest priority
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


}

void TIM4_IRQHandler(void) {
  if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)
  {
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    // calculate motor  speed or else with CCR1 values

  }
  else if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)
  {
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);


	uint16_t newhallpos = ((GPIO_ReadInputData(GPIOB)>>6) & 0x0007);

	if (newhallpos == hallpos) return;


	hallpos = newhallpos;
	pwm_Commute(hallpos);

  } else {
    ;
  }
}
