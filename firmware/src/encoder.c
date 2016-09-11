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

#include "encoder.h"
#include "usart.h"
#include "input.h"
#include "pwm.h"
#include "utils.h"
#include "adc.h"
#include "math.h"

volatile int32_t encoder_count;

uint16_t encoder_shaft_pos; //this is the shaft position as encoder counts
uint16_t encoder_commutation_pos; //this is shaft position from the beginning of current commuatiton sequence.

int16_t sine_table[4096]; //the required length of sine_table is encoder_ppr/number_of_poles, so this allows quite high ppr encoder..

volatile uint16_t encoder_lastCount;

uint16_t commutation_length; //how many encoder steps there are per commutation circle (i.e encoder_ppr/motor_poles)


void buildCommutationTable()
{
	uint16_t i;
	commutation_length = s.encoder_PPR/s.encoder_poles;

	for(i=0;i<commutation_length;i++)
	{
		sine_table[i] = (int16_t)(sinf((float)i/(float)commutation_length*2*M_PI)*(float)SINE_TABLE_MAX);
	}
}

/**
 *this function sets the current sine values for each phase based on current commutation position and direction of drive
 */

uint16_t getCommutationPos(uint8_t phase)
{
	//if driving forward, the commutation must lead the actual position by 90 degrees
	//if driving backward, the commutation must lag the actual position by 90 degrees
	int32_t tmp;
	if(dir)
	{
		tmp = (encoder_commutation_pos - commutation_length/4 + commutation_length*phase/3);
		if (tmp<0)
		{
			tmp+=commutation_length;
		}
		return (uint16_t)(tmp % commutation_length);

	}
	else
	{
		tmp= (encoder_commutation_pos + commutation_length/4 + commutation_length*phase/3);
		if (tmp<0)
		{
			tmp+=commutation_length;
		}
		return (uint16_t)(tmp % commutation_length);

	}
}

void forcedInitialization()
{
	//apply positive voltage (PWM) to phase B and negative to phase C. (Commutation table starts from that position)
	if(motor_running)
		return;


	TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_ForcedAction_InActive);
	TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
	TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);

	TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_OCMode_PWM1);
	TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable);
	TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Enable);

	TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_ForcedAction_Active);
	TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Enable);
	uint16_t i;
	for(i=2000;i<4000;i++)
	{
		//increase duty until ADC kicks in
		if(i>max_duty)
		{
			TIM1->CCR1=max_duty;
			TIM1->CCR2=max_duty;
			TIM1->CCR3=max_duty;
		}
		else
		{
			TIM1->CCR1=i;
			TIM1->CCR2=i;
			TIM1->CCR3=i;
		}
		delay_ms(1);
	}
	delay_ms(1000);
	TIM_SetCounter(ENCODER_TIM,0);
	encoder_count=0;
	encoder_lastCount=0;
	encoder_shaft_pos=0;

	delay_ms(1);
	TIM1->CCR1=0;
	TIM1->CCR2=0;
	TIM1->CCR3=0;


}

void initEncoder()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
if( ENCODER_TIM==TIM2)
{
#ifndef ENCODER_TIM2_REMAP
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#else
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);


#endif

} else
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

	TIM_DeInit(ENCODER_TIM);

	TIM_TimeBaseInitTypeDef timerInitStructure;
	TIM_TimeBaseStructInit(&timerInitStructure);
	timerInitStructure.TIM_Prescaler = 0;
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = 0xFFFF;
	timerInitStructure.TIM_ClockDivision = 0;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(ENCODER_TIM, &timerInitStructure);


	TIM_EncoderInterfaceConfig (ENCODER_TIM, TIM_EncoderMode_TI12,
								  TIM_ICPolarity_Rising,
								  TIM_ICPolarity_Rising);



	TIM_Cmd(ENCODER_TIM, ENABLE);


	buildCommutationTable();
	forcedInitialization();






}

void getEncoderCount()
{
	uint16_t now = ENCODER_TIM->CNT;
	int16_t delta = (int16_t)(now - encoder_lastCount);
	encoder_lastCount = now;

	encoder_count += delta;

	int16_t shaft_pos_tmp = encoder_count % s.encoder_PPR;
	if(shaft_pos_tmp < 0)
		encoder_shaft_pos = s.encoder_PPR + shaft_pos_tmp;
	else
		encoder_shaft_pos = shaft_pos_tmp;

	encoder_commutation_pos = encoder_shaft_pos % commutation_length;

}
