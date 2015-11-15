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
#include "hall.h"
#include "utils.h"


uint16_t encoder_lastCount;
//uint16_t findNextEncoderCommutationCNT(int8_t dir);
void buildCommutationTable()
{
	uint16_t i;
	//char buf[10];
	for(i=0;i<s.encoder_PPR;i++)
	{
		encoder_commutation_table[i] = commutation_sequence[(i*s.encoder_poles*6/s.encoder_PPR) % 6];
		//sprintf(buf,"%d\n\r",encoder_commutation_table[i]);
		//usart_sendStr(buf);
	}
}

/* THIS IS OLD (INTERRUPT BASED) METHOD OF COMMUTATION
void buildCommutationTable()
{
	uint16_t i;
	char buf[10];
	for(i = 0;i<s.encoder_poles*6;i++)
	{
		encoder_commutation_table[i] = (i * s.encoder_PPR) / s.encoder_poles / 6;
		sprintf(buf,"%d\n\r",encoder_commutation_table[i]);
		usart_sendStr(buf);
	}
	encoder_commutation_table[i]=UINT16_MAX;
}

*/
void forcedInitialization()
{
	//during initialization the positive voltage will be applied to Channel1 and negative to Channel3. This will move the rotor to commutation position 1
	if(motor_running)
		return;


	TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_OCMode_PWM1);
	TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
	TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);

	TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_ForcedAction_InActive);
	TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Enable);

	TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_ForcedAction_Active);
	TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Enable);
	uint16_t i;
	for(i=0;i<1000;i++)
	{
		TIM1->CCR1=i;
		TIM1->CCR2=i;
		TIM1->CCR3=i;
		delay_ms(1);
	}
	delay_ms(10);
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

/*
	if(s.commutationMethod == commutationMethod_Encoder)
	{
		//initialize commutation interrupts for Encoder TIM
		TIM_OCInitTypeDef       TIM2_OC;
		TIM2_OC.TIM_OCMode      = TIM_OCMode_Inactive;                // Output compare toggling mode
		TIM2_OC.TIM_OutputState = TIM_OutputState_Disable;           // Enabling the Output Compare state
		TIM2_OC.TIM_OCPolarity  = TIM_OCPolarity_High;               // Reverse polarity
		TIM2_OC.TIM_Pulse       = 20000;                       // Initialize to some large value, will be reinitialized
		TIM_OC3Init(TIM2, &TIM2_OC);                                // Initializing Output Compare 1 structure
		TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);          // Disabling Ch.1 Output Compare preload
		TIM2_OC.TIM_Pulse       = 22000;                       // Initialize to some large value, will be reinitialized
		TIM_OC4Init(TIM2, &TIM2_OC);                                // Initializing Output Compare 1 structure
		TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Disable);          // Disabling Ch.1 Output Compare preload


		TIM_ClearFlag(TIM2, TIM_FLAG_CC3);
		TIM_ClearFlag(TIM2, TIM_FLAG_CC4);

		TIM_ITConfig(TIM2, TIM_IT_CC3 | TIM_IT_CC4, ENABLE);


		// we use preemption interrupts here,  BLDC Bridge switching and
		// Hall has highest priority
		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		buildCommutationTable();
		forcedInitialization();


		TIM2->CCR3 = -1;
		TIM2->CCR4 = findNextEncoderCommutationCNT(1);
		TIM_ITConfig(TIM2, TIM_IT_CC3 | TIM_IT_CC4, ENABLE);


	}
	*/
	if(s.commutationMethod == commutationMethod_Encoder)
	{
		buildCommutationTable();
		forcedInitialization();
	}
	if(s.commutationMethod == commutationMethod_HALL)
	{
	encoder_count=0;
	encoder_lastCount=0;
	encoder_shaft_pos=0;



	}


}

/*
uint16_t findCurrentEncoderCommutationPos()
{
	uint16_t i;
	uint16_t pos;

	for(i=s.encoder_poles*6;i>=0;i--)
	{
		if(encoder_shaft_pos >= encoder_commutation_table[i])
		{
			pos = i % 6;
			return pos;
		}
	}
	//newer should end up here
	return 0;


}
uint16_t findNextEncoderCommutationPos(int8_t dir)
{

	uint16_t i;
	int16_t add;
	uint16_t pos;
	uint16_t total = s.encoder_poles*6;
	for(i=s.encoder_poles*6;i>=0;i--)
	{
		if(encoder_shaft_pos >= encoder_commutation_table[i])
		{
			if(dir>0)
				add = i+1;
			else
				add = i-1;
			if(add==s.encoder_poles*6)
				add = 0;
			else if(add==-1)
				add =((s.encoder_poles*6)-1);
			return (encoder_full_rounds*s.encoder_PPR + encoder_commutation_table[add]);
		}
	}
	//newer should end up here
	return 0;


}

uint16_t findNextEncoderCommutationCNT(int8_t dir)
{

	uint16_t i;
	int16_t add;
	uint16_t pos;
	uint16_t total = s.encoder_poles*6;
	int16_t tmp;
	uint16_t CNT_tmp,CNT_delta;
	for(i=s.encoder_poles*6;i>=0;i--)
	{
		if(encoder_shaft_pos >= encoder_commutation_table[i])
		{
			if(dir>0)
			{
				add = i+1;
				if(add>=s.encoder_poles*6)
				{
					add = 0;
					CNT_delta = s.encoder_PPR-encoder_shaft_pos;
				}
				else
				{
					CNT_delta = encoder_commutation_table[add]-encoder_shaft_pos;
				}

			}
			else
			{
				add = i-1;
				if(add<=-1)
				{
					add =((s.encoder_poles*6)-1);
					CNT_delta = encoder_commutation_table[add]-s.encoder_PPR;
				}
				else
				{
					CNT_delta = encoder_commutation_table[add]-encoder_shaft_pos;
				}
			}


			tmp = ENCODER_TIM->CNT + CNT_delta;
			//if(tmp<0) tmp = 65535-tmp;
			//else if (tmp>65535) tmp -= 65535;
			return (uint16_t)tmp;
		}
	}

	//newer should end up here
	return 0;

}
*/
void getEncoderCount()
{
	uint16_t now = ENCODER_TIM->CNT;
	int16_t delta = (int16_t)(now - encoder_lastCount);
	encoder_lastCount = now;

	encoder_count += delta;

	//encoder_full_rounds = encoder_count / s.encoder_PPR;
	int16_t shaft_pos_tmp = encoder_count % s.encoder_PPR;
	if(shaft_pos_tmp < 0)
		encoder_shaft_pos = s.encoder_PPR + shaft_pos_tmp;
	else
		encoder_shaft_pos = shaft_pos_tmp;


}
/*
void TIM2_IRQHandler(void) {
  int8_t currentEncPos = findCurrentEncoderCommutationPos();
  if (TIM_GetITStatus(TIM2, TIM_IT_CC3) != RESET)
  {
	  getEncoderCount();
	  currentEncPos--;
	  if(currentEncPos<0) currentEncPos=5;
	  TIM2->CCR3 = findNextEncoderCommutationCNT(0);
	  TIM2->CCR4 = TIM2->CNT+1;
	  TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);
	  pwm_Commute(commutation_sequence[currentEncPos]);
	  //usart_sendStr("ENC commuted CCW: ");
	  //usart_sendChar(commutation_sequence[findCurrentEncoderCommutationPos(0)] + 48);
	  //usart_sendStr("\n\r");

  }
  else if (TIM_GetITStatus(TIM2, TIM_IT_CC4) != RESET)
  {
	  getEncoderCount();
	  currentEncPos++;
	  if(currentEncPos>5) currentEncPos=0;
	  TIM2->CCR4 = findNextEncoderCommutationCNT(1);
	  TIM2->CCR3 = TIM2->CNT-1;
	  TIM_ClearITPendingBit(TIM2, TIM_IT_CC4);
	  pwm_Commute(commutation_sequence[currentEncPos]);
	  //usart_sendStr("ENC commuted CW: ");
	  //usart_sendChar(commutation_sequence[findCurrentEncoderCommutationPos(1)] + 48);
	  //usart_sendStr("\n\r");
  } else {
    ; // this should not happen
  }

}*/


