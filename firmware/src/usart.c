/*
 * usart.c
 *
 *  Created on: Oct 28, 2015
 *      Author: pekka
 */

#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_rcc.h>
#include <string.h>
#include "usart.h"
#include "pid.h"
#include "pwm.h"
#include "encoder.h"
#include "adc.h"
#include "hall.h"
#include "configuration.h"
char recvbuffer[255];
char recvctr;
void initUSART(uint16_t baud)
{
	recvctr=0;
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
void parseUsart()
{
	const char delimiters[] = " ";
	char *token;
	char *param;
	char *value;
	if(strstr(recvbuffer, "SET")!=NULL)
	{
	  token = strtok(recvbuffer,delimiters); //first param
	  param = strtok(NULL,delimiters);
	  value = strtok(NULL,delimiters);
	  if(param!=NULL && value!=NULL)
		  setConfig(param,atoi(value));

	}
	//too empty message for anything. Send status
	if(recvctr < 3)
	{
		char buf[100];
		sprintf(buf, "Count: %d, Hall: %d, ADC: %d\n\r",encoder_count, hallpos, ADC_value);

		usart_sendStr(buf);
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
			USART_SendData(USART3, recvbuffer[recvctr]);
			while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
			{

			}
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
