/*
 	bldc-drive Cheap and simple brushless DC motor driver designed for CNC applications using STM32 microcontroller.
	Copyright (C) 2015 Pekka Roivainen

	Original PWM commutation tables and ideas on PWM commutation from a great article in http://www.mikrocontroller.net/articles/STM32_BLDC_Control_with_HALL_Sensor

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
#include "pwm.h"
#include "input.h"
#include "hall.h"
#include "encoder.h"
#include "configuration.h"
#include "adc.h"

#define FALSE 0
#define TRUE 1

//volatile uint8_t errorInCommutation;


static const uint8_t BLDC_BRIDGE_STATE_BACKWARD[8][6] =   // Motor step
{
   { FALSE,FALSE   ,   FALSE,FALSE   ,  FALSE,FALSE },  //  //000
   { FALSE,FALSE, TRUE,FALSE, FALSE,TRUE },
   { TRUE,FALSE, FALSE,TRUE, FALSE,FALSE },
   { TRUE,FALSE, FALSE,FALSE, FALSE,TRUE },
   { FALSE,TRUE, FALSE,FALSE, TRUE,FALSE },
   { FALSE,TRUE, TRUE,FALSE, FALSE,FALSE },
   { FALSE,FALSE, FALSE,TRUE, TRUE,FALSE },
   { FALSE,FALSE   ,   FALSE,FALSE   ,  FALSE,FALSE },  //  //111
};

static const uint8_t BLDC_BRIDGE_STATE_VORWARD[8][6] =   // Motor step
{
   { FALSE,FALSE   ,   FALSE,FALSE   ,  FALSE,FALSE },  // 0 //000
   { FALSE,FALSE, FALSE,TRUE, TRUE,FALSE },
   { FALSE,TRUE, TRUE,FALSE, FALSE,FALSE },
   { FALSE,TRUE, FALSE,FALSE, TRUE,FALSE },
   { TRUE,FALSE, FALSE,FALSE, FALSE,TRUE },
   { TRUE,FALSE, FALSE,TRUE, FALSE,FALSE },
   { FALSE,FALSE, TRUE,FALSE, FALSE,TRUE },
   { FALSE,FALSE   ,   FALSE,FALSE   ,  FALSE,FALSE },  // 0 //111
};
/*
static const uint8_t BLDC_BRIDGE_STATE_VORWARD2[8][6] =   // Motor step
{
{ FALSE,FALSE   ,   FALSE,FALSE   ,  FALSE,FALSE },  //  //000
{ FALSE,TRUE, FALSE,FALSE, TRUE,FALSE },
{ FALSE,FALSE, TRUE,FALSE, FALSE,TRUE },
{ FALSE,TRUE, TRUE,FALSE, FALSE,FALSE },
{ TRUE,FALSE, FALSE,TRUE, FALSE,FALSE },
{ FALSE,FALSE, FALSE,TRUE, TRUE,FALSE },
{ TRUE,FALSE, FALSE,FALSE, FALSE,TRUE },

{ FALSE,FALSE   ,   FALSE,FALSE   ,  FALSE,FALSE },  // 0 //111
};

static const uint8_t BLDC_BRIDGE_STATE_BACKWARD2[8][6] =   // Motor step
{
{ FALSE,FALSE   ,   FALSE,FALSE   ,  FALSE,FALSE },  //  //000
{ FALSE,TRUE, TRUE,FALSE, FALSE,FALSE },
{ TRUE,FALSE, FALSE,FALSE, FALSE,TRUE },
{ FALSE,FALSE, TRUE,FALSE, FALSE,TRUE },
{ FALSE,FALSE, FALSE,TRUE, TRUE,FALSE },
{ FALSE,TRUE, FALSE,FALSE, TRUE,FALSE },
{ TRUE,FALSE, FALSE,TRUE, FALSE,FALSE },

{ FALSE,FALSE   ,   FALSE,FALSE   ,  FALSE,FALSE },  // 0 //111
};
*/
void UpdatePWMChannels(uint8_t BL1,uint8_t BL2,uint8_t BL3,uint8_t BH1,uint8_t BH2,uint8_t BH3);
void initPWM()

{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

	//initialize Tim1 PWM outputs
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	// Time Base configuration
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = BLDC_CHOPPER_PERIOD;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	// Channel 1, 2, 3 – set to PWM mode - all 6 outputs

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0; // initialize to zero output

	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;

	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);
	TIM_OC4Init(TIM1, &TIM_OCInitStructure); //for ADC

	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;


	// DeadTime[ns] = value * (1/SystemCoreFreq) (on 72MHz: 7 is 98ns)
	TIM_BDTRInitStructure.TIM_DeadTime = BLDC_NOL;

	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;

	//no break functionality
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;

	TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);


	//Commutation event mapped to TIM4 - We are not using commute event, but interrupt from Hall timer directly to commute.
	//Not optimal solution?
	//TIM_SelectInputTrigger(TIM1, TIM_TS_ITR3);


	TIM_ITConfig(TIM1, TIM_IT_CC4, ENABLE); //adc sampling interrupt
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);


	TIM_Cmd(TIM1, ENABLE);
	 // enable motor timer main output (the bridge signals)
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	pwm_motorStop();

}


void TIM1_CC_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_CC4) != RESET)
	{
		 ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		TIM_ClearITPendingBit(TIM1, TIM_IT_CC4);
	}
}




void pwm_setDutyCycle(uint16_t duty)
{
	if(!motor_running)
		return;
	uint16_t d = duty;
	if(d>MAX_DUTY) d = MAX_DUTY; //this is absolute MAX
	if(d>max_duty) d = max_duty; //this is maximum set by ADC current limiting
	TIM1->CCR1 = d;
	TIM1->CCR2 = d;
	TIM1->CCR3 = d;
	TIM1->CCR4 = d>>1; //get the ADC conversion on the half duty cycle.

}
void pwm_motorStart()
{
	motor_running=1;

	pwm_InitialBLDCCommutation();
	TIM_ITConfig(TIM4, TIM_IT_CC1 | TIM_IT_CC2, ENABLE); //enable HALL interrupts again

}
void pwm_motorStop()
{
	motor_running=0;
	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	TIM1->CCR3 = 0;
	TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
	TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);
	TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);

	TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);
	TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Disable);
	TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Disable);
	TIM_ITConfig(TIM4, TIM_IT_CC1 | TIM_IT_CC2, DISABLE); //disable HALL interrupts, no commutation!

}
void pwm_InitialBLDCCommutation()
{
	uint16_t newhallpos;
	if(s.commutationMethod == commutationMethod_HALL)
	{
		//this function is called when forced commutation is required, eg when starting or when anomalies are detected.
		newhallpos = ((GPIO_ReadInputData(GPIOB)>>6) & 0x0007);
		hallpos = newhallpos;
		pwm_Commute(hallpos);
		return;

	}
	else
	{
		encoder_commutation_pos = encoder_commutation_table[encoder_shaft_pos];
		pwm_Commute(encoder_commutation_pos);
		return;
	}



}

void pwm_Commute(uint8_t comm_pos)
{
	uint8_t BH1,BH2,BH3,BL1,BL2,BL3;

	if(dir)
		{
		  BH1 = BLDC_BRIDGE_STATE_VORWARD[comm_pos][0];
		  BL1 = BLDC_BRIDGE_STATE_VORWARD[comm_pos][1];

		  BH2 = BLDC_BRIDGE_STATE_VORWARD[comm_pos][2];
		  BL2 = BLDC_BRIDGE_STATE_VORWARD[comm_pos][3];

		  BH3 = BLDC_BRIDGE_STATE_VORWARD[comm_pos][4];
		  BL3 = BLDC_BRIDGE_STATE_VORWARD[comm_pos][5];
		}
		else
		{
		  BH1 = BLDC_BRIDGE_STATE_BACKWARD[comm_pos][0];
		  BL1 = BLDC_BRIDGE_STATE_BACKWARD[comm_pos][1];

		  BH2 = BLDC_BRIDGE_STATE_BACKWARD[comm_pos][2];
		  BL2 = BLDC_BRIDGE_STATE_BACKWARD[comm_pos][3];

		  BH3 = BLDC_BRIDGE_STATE_BACKWARD[comm_pos][4];
		  BL3 = BLDC_BRIDGE_STATE_BACKWARD[comm_pos][5];
		}
	UpdatePWMChannels(BL1,BL2,BL3,BH1,BH2,BH3);

}
void UpdatePWMChannels(uint8_t BL1,uint8_t BL2,uint8_t BL3,uint8_t BH1,uint8_t BH2,uint8_t BH3)
{
//THIS NEEDS OPTIMIZATION!
	#define DRIVEMODE 1
#if DRIVEMODE==1
	// **** this is with active freewheeling ****
		  // Bridge FETs for Motor Phase U
		  if (BH1) {

			// PWM at low side FET of bridge U
			// active freewheeling at high side FET of bridge U
			// if low side FET is in PWM off mode then the hide side FET
			// is ON for active freewheeling. This mode needs correct definition
			// of dead time otherwise we have shoot-through problems

			TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_OCMode_PWM1);

			TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);

			TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);

		  } else {

			// Low side FET: OFF
			TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);

			if (BL1){

			 // High side FET: ON
			 TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_ForcedAction_Active);

			  TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);

			} else {

			  // High side FET: OFF
			  TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);

			}

		  }

		  // Bridge FETs for Motor Phase V

		  if (BH2) {
			TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_OCMode_PWM1);
			TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable);
			TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Enable);
		  } else {
			TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Disable);

			if (BL2){
			  TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_ForcedAction_Active);
			  TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Enable);
			} else {
			  TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Disable);
			}
		  }

		  // Bridge FETs for Motor Phase W

		  if (BH3) {
			TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_OCMode_PWM1);
			TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable);
			TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Enable);
		  } else {
			TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Disable);

		  if (BL3){
			  TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_ForcedAction_Active);
			  TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Enable);
			} else {
			  TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Disable);
			}
		  }

#else

		uint16_t positive_oc_mode = TIM_OCMode_PWM1;
		uint16_t negative_oc_mode = TIM_OCMode_Inactive;

		uint16_t positive_highside = TIM_CCx_Enable;
		uint16_t positive_lowside = TIM_CCxN_Enable;

		uint16_t negative_highside = TIM_CCx_Enable;
		uint16_t negative_lowside = TIM_CCxN_Enable;
		 if(!BH1&&!BL1) //0
		 {
			TIM_SelectOCxM(TIM1, TIM_Channel_1, TIM_OCMode_Inactive);
			TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
			TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);
		 }
		 else if(BH1) //+
		 {
			TIM_SelectOCxM(TIM1, TIM_Channel_1, positive_oc_mode);
			TIM_CCxCmd(TIM1, TIM_Channel_1, positive_highside);
			TIM_CCxNCmd(TIM1, TIM_Channel_1, positive_lowside);
		 }
		 else if(BL1) //-
		 {
			TIM_SelectOCxM(TIM1, TIM_Channel_1, negative_oc_mode);
			TIM_CCxCmd(TIM1, TIM_Channel_1, negative_highside);
			TIM_CCxNCmd(TIM1, TIM_Channel_1, negative_lowside);
		 }

		 //CHANNEL2
		 if(!BH2&&!BL2) //0
		 {
			TIM_SelectOCxM(TIM1, TIM_Channel_2, TIM_OCMode_Inactive);
			TIM_CCxCmd(TIM1, TIM_Channel_2, TIM_CCx_Enable);
			TIM_CCxNCmd(TIM1, TIM_Channel_2, TIM_CCxN_Disable);
		 }
		 else if(BH2) //+
		 {
			TIM_SelectOCxM(TIM1, TIM_Channel_2, positive_oc_mode);
			TIM_CCxCmd(TIM1, TIM_Channel_2, positive_highside);
			TIM_CCxNCmd(TIM1, TIM_Channel_2, positive_lowside);
		 }
		 else if(BL2) //-
		 {
			TIM_SelectOCxM(TIM1, TIM_Channel_2, negative_oc_mode);
			TIM_CCxCmd(TIM1, TIM_Channel_2, negative_highside);
			TIM_CCxNCmd(TIM1, TIM_Channel_2, negative_lowside);
		 }

		 //CHANNEL3
		 if(!BH3&&!BL3) //0
		 {
			TIM_SelectOCxM(TIM1, TIM_Channel_3, TIM_OCMode_Inactive);
			TIM_CCxCmd(TIM1, TIM_Channel_3, TIM_CCx_Enable);
			TIM_CCxNCmd(TIM1, TIM_Channel_3, TIM_CCxN_Disable);
		 }
		 else if(BH3) //+
		 {
			TIM_SelectOCxM(TIM1, TIM_Channel_3, positive_oc_mode);
			TIM_CCxCmd(TIM1, TIM_Channel_3, positive_highside);
			TIM_CCxNCmd(TIM1, TIM_Channel_3, positive_lowside);
		 }
		 else if(BL3) //-
		 {
			TIM_SelectOCxM(TIM1, TIM_Channel_3, negative_oc_mode);
			TIM_CCxCmd(TIM1, TIM_Channel_3, negative_highside);
			TIM_CCxNCmd(TIM1, TIM_Channel_3, negative_lowside);
		 }
#endif

}

