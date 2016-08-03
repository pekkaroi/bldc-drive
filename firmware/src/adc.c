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

#include <stm32f10x_adc.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_rcc.h>

#include "adc.h"
#include "pwm.h"


volatile uint16_t ADC_value;
volatile uint16_t max_duty;

void initADC()
{

	max_duty=MAX_DUTY;
	GPIO_InitTypeDef GPIO_InitStructure;
	//--Enable ADC1 and GPIOA--
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

	//==Configure ADC pins as analog inputs==
	GPIO_StructInit(&GPIO_InitStructure); // Reset init structure, if not it can cause issues...
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; //PA4 = bus measurement
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	ADC_InitTypeDef ADC_InitStructure;
	//ADC1 configuration

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	//We will convert multiple channels
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	//select continuous conversion mode
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//!
	//select external triggering
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	//right 12-bit data alignment in ADC data register
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	//load structure values to control and status registers
	ADC_Init(ADC1, &ADC_InitStructure);
	//wake up temperature sensor
	//ADC_TempSensorVrefintCmd(ENABLE);
	//configure each channel
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_41Cycles5);
	//Enable ADC1
	ADC_Cmd(ADC1, ENABLE);
	//enable DMA for ADC
	ADC_DMACmd(ADC1, ENABLE);
	//Enable ADC1 reset calibration register
	ADC_ResetCalibration(ADC1);
	//Check the end of ADC1 reset calibration register
	while(ADC_GetResetCalibrationStatus(ADC1));
	//Start ADC1 calibration
	ADC_StartCalibration(ADC1);
	//Check the end of ADC1 calibration
	while(ADC_GetCalibrationStatus(ADC1));
	//ADC_ExternalTrigConvCmd(ADC1, ENABLE);

	//ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
	//NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
	//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	//NVIC_Init(&NVIC_InitStructure);




	//enable DMA1 clock
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	//create DMA structure
	DMA_InitTypeDef  DMA_InitStructure;
	//reset DMA1 channe1 to default values;
	DMA_DeInit(DMA1_Channel1);
	//channel will be used for memory to memory transfer
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	//setting normal mode (non circular)
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	//medium priority
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	//source and destination data size word=32bit
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	//automatic memory destination increment enable.
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	//source address increment disable
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	//Location assigned to peripheral register will be source
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	//chunk of data to be transfered
	DMA_InitStructure.DMA_BufferSize = 1;
	//source and destination start addresses
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_value;
	//send values to DMA registers
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	// Enable DMA1 Channel Transfer Complete interrupt
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
	DMA_Cmd(DMA1_Channel1, ENABLE); //Enable the DMA1 - Channel1

	//Enable DMA1 channel IRQ Channel */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//ADC sampling timer
/*
	RCC_APB1PeriphClockCmd(ADC_TIM_PERIPH, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(ADC_TIM, &TIM_TimeBaseStructure);

	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 250;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;
	TIM_OC4Init(ADC_TIM, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(ADC_TIM, TIM_OCPreload_Enable);


	TIM_ARRPreloadConfig(TIM2, ENABLE);
	TIM_CCPreloadControl(TIM2, ENABLE);

	// PWM outputs have to be enabled in order to trigger ADC on CCx

	//TIM_ITConfig(TIM4, TIM_IT_CC4, ENABLE);
	// TIM1 Master and TIM8 slave
	TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);
	TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);
	TIM_SelectInputTrigger(ADC_TIM, TIM_TS_ITR0);
	TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Reset);
	TIM_SelectOutputTrigger(ADC_TIM, TIM_TRGOSource_OC4Ref);
	TIM_Cmd(ADC_TIM, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
*/

}

void ADC1_2_IRQHandler(void) {
  if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET)
  {

	  ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

  }
}
/*
void TIM3_IRQHandler(void) {
  if (TIM_GetITStatus(TIM3, TIM_IT_CC4) != RESET)
  {
	//  TM_GPIO_TogglePinValue(GPIOA, GPIO_Pin_12);
	  TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);
    // calculate motor  speed or else with CCR1 values

  }
}
*/


void DMA1_Channel1_IRQHandler(void) {
  if(DMA_GetITStatus(DMA1_IT_TC1)) {
    //while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) { };
    //while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET) { };
    //GPIO_SetBits(GPIOA,GPIO_Pin_4);       // CS

	DMA_ClearITPendingBit(DMA1_IT_GL1);//0x2503
	DMA_Cmd(DMA1_Channel1, ENABLE);
	if(ADC_value>s.max_current && max_duty>1)
	{
		max_duty -= 2;
		return;
	}
	if(ADC_value<=s.max_current && max_duty<MAX_DUTY)
	{
		max_duty ++;
		return;
	}




  }
}
