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
#include <stm32f10x_usart.h>
#include <stm32f10x_rcc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "usart.h"
#include "pid.h"
#include "pwm.h"
#include "encoder.h"
#include "adc.h"
//#include "hall.h"
#include "configuration.h"

char txbuffer[255];
volatile uint8_t serial_stream_enabled;


char recvbuffer[255];
uint8_t recvctr;
DMA_InitTypeDef DMA_InitStructure;

void initUSART(uint16_t baud)
{
	recvctr=0;
	serial_stream_enabled=0;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;


	if(USART == USART1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO |
			                        RCC_APB2Periph_GPIOA, ENABLE);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);


		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}

	else if(USART==USART3)
	{

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		/* Configure USART3_Rx as input floating                */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}

	USART_InitStructure.USART_BaudRate = baud*100;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART, &USART_InitStructure);

	USART_ITConfig(USART, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART, ENABLE);

	//DMA for USART TX!
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	  /* USARTy_Tx_DMA_Channel (triggered by USARTy Tx event) Config */

	DMA_DeInit(DMA1_Channel2);
	DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004804;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)txbuffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 255;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);

	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);

	//DMA_Cmd(DMA1_Channel2, ENABLE);


}
void usart_startDMA(uint16_t len)
{
	DMA_DeInit(DMA1_Channel2);
	DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004804;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)txbuffer;
	DMA_InitStructure.DMA_BufferSize = len;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel2, ENABLE);
}
void usart_sendChar(char chr)
{
	USART_SendData(USART, chr);
	while (USART_GetFlagStatus(USART, USART_FLAG_TXE) == RESET);
}

void usart_sendStr(char *str)
{
	while (*str != '\0')
	{
	usart_sendChar(*str);
	str ++;
	}
}
void usart_send_stream()
{

//	while (DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET)
//	{
//	}
	uint16_t len = sprintf(txbuffer, "STR:%d;%d;%d;%d;%d;%d;%d;%d\r",0,(int)encoder_count,(int)pid_requested_position,(int)pid_last_requested_position_delta,(int)position_error,(int)ADC_value,(int)TIM1->CCR1,pid_integrated_error);
	usart_startDMA(len);
}
void parseUsart()
{
	const char delimiters[] = " ";

	char *param;
	char *value;
	if(strstr(recvbuffer, "STREAM")!=NULL)
	{
		strtok(recvbuffer,delimiters); //first param
		value = strtok(NULL,delimiters);
		if(strstr(value,"START"))
		{
			serial_stream_enabled=1;
			usart_send_stream();
		}
		else
			serial_stream_enabled=0;
	}
	if(strstr(recvbuffer, "SET")!=NULL)
	{
	  strtok(recvbuffer,delimiters); //first param
	  param = strtok(NULL,delimiters);
	  value = strtok(NULL,delimiters);
	  if(param!=NULL && value!=NULL)
		  setConfig(param,atoi(value));

	}
	if(strstr(recvbuffer, "SAVE")!=NULL)
	{
		usart_sendStr("Saving:\n\r");
		printConfiguration();
		writeConfig(s);
		usart_sendStr("SAVE OK\n\r");
	}
	if(strstr(recvbuffer, "GET")!=NULL)
	{
		//getConfig();
		printConfiguration();
	}


	if(recvctr < 3)
	{

		uint16_t len =sprintf(txbuffer, "Count: %d, Hall: %d, error: %d, max_error: %d\n\r",(int)encoder_count, (int)0, (int)position_error, (int)max_error);
		max_error=0;
		usart_startDMA(len);


	}
}
void USART3_IRQHandler(void)
{
	char in;
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
    	in = (char)USART_ReceiveData(USART3);
    	recvbuffer[recvctr] = in;

    	if(in=='\r')
    	{
    		parseUsart();
    		recvctr=0;

    	}
    	else
    	{
			//USART_SendData(USART3, recvbuffer[recvctr]);
			//while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
			//{

			//}
    		//any received character will stop the stream.
    		serial_stream_enabled=0;
			recvctr++;
    	}

    }

}

void USART1_IRQHandler(void)
{
	char in;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
    	in = (char)USART_ReceiveData(USART1);
    	recvbuffer[recvctr] = in;

    	if(in=='\r')
    	{
    		parseUsart();
    		recvctr=0;
    	}
    	else
    	{
			USART_SendData(USART1, recvbuffer[recvctr]);
			while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
			{

			}
    	}
    	recvctr++;
    }

}
