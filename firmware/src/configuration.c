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


#include "configuration.h"
#include "eeprom.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#define EADDR_IS_INITIALIZED 0x0001
#define EADDR_CONFIG_START 0x0002

void writeConfig()
{



	uint16_t i;
	int16_t *ptr = (int16_t *)&s;
	for(i=0; i<sizeof(servoConfig)/2;i++)
	{
		if(EE_WriteVariable(EADDR_CONFIG_START+i, *ptr)!=FLASH_COMPLETE)
		{
			usart_sendStr("Flash Error\n\r");
		}
		ptr++;
	}


}
void getConfig()
{
	FLASH_Unlock();

	uint16_t i;

	uint16_t *ptr;
	EE_Init();
	EE_ReadVariable(EADDR_IS_INITIALIZED,&i);
	if(i != 0x5253)
	{
		//empty or corrupted EEPROM detected: write default config
		//EE_Format();
		EE_WriteVariable(EADDR_IS_INITIALIZED, 0x5253);
		s.commutationMethod = commutationMethod_HALL;
		s.inputMethod = inputMethod_stepDir;
		s.encoder_PPR = 4000;
		s.encoder_poles = 4;
		s.encoder_counts_per_step = 10;
		s.pid_Kp = 10;
		s.pid_Ki = 0;
		s.pid_Kd = 0;
		s.usart_baud = 1152;
		writeConfig(s);
		return;

	}
	ptr = (uint16_t *)&s;
	for(i=0; i<sizeof(servoConfig)/2;i++)
	{

		if(EE_ReadVariable(EADDR_CONFIG_START+i, ptr)!=0)
			usart_sendStr("Flash Error\n\r");
		ptr++;
	}

	return;

}

void setConfig(char* param, int16_t value)
{
	if (strstr( param, "commutationMethod" ) != NULL)
	{
		s.commutationMethod = (uint16_t)value;
		//writeConfig();

		//this is taken into account on next boot
		usart_sendStr("SET commutationMethod to ");
		usart_sendChar(value + 48);
		usart_sendStr("\n\r");
	}
	if (strstr( param, "inputMethod" ) != NULL)
	{
		s.inputMethod = (uint16_t)value;
		//writeConfig();
		//this is taken into account on next boot

		usart_sendStr("SET OK");
	}
	if (strstr( param, "encoder_PPR" ) != NULL)
	{
		s.encoder_PPR = (uint16_t)value;

		//writeConfig();

		usart_sendStr("SET OK");
	}
	if (strstr( param, "encoder_poles" ) != NULL)
	{
		s.encoder_poles = (uint16_t)value;
		//writeConfig();

		usart_sendStr("SET OK");
	}
	if (strstr( param, "encoder_counts_per_step" ) != NULL)
	{
		s.encoder_counts_per_step = (uint16_t)value;
		//writeConfig();

		usart_sendStr("SET OK");
	}
	if (strstr( param, "pid_Kp" ) != NULL)
	{
		s.pid_Kp = (int16_t)value;
		//writeConfig();

		usart_sendStr("SET OK");
	}
	if (strstr( param, "pid_Ki" ) != NULL)
	{
		s.pid_Ki = (int16_t)value;
		//writeConfig();

		usart_sendStr("SET OK");
	}
	if (strstr( param, "pid_Kd" ) != NULL)
	{
		s.pid_Kd = (int16_t)value;
		//writeConfig();

		usart_sendStr("SET OK");
	}
	if (strstr( param, "usart_baud" ) != NULL)
	{
		s.usart_baud = (uint16_t)value;
		//writeConfig();
		//this is taken into account on next boot
		usart_sendStr("SET OK");
	}
}
void printConfiguration()
{
	char buf[50];
	sprintf(buf,"commutationMethod: %d\n\r",s.commutationMethod);
	usart_sendStr(buf);
	sprintf(buf,"inputMethod: %d\n\r",s.inputMethod);
	usart_sendStr(buf);
	sprintf(buf,"encoder_PPR: %d\n\r",s.encoder_PPR);
	usart_sendStr(buf);
	sprintf(buf,"encoder_poles: %d\n\r",s.encoder_poles);
	usart_sendStr(buf);
	sprintf(buf,"encoder_counts_per_step: %d\n\r",s.encoder_counts_per_step);
	usart_sendStr(buf);
	sprintf(buf,"pid_Kp: %d\n\r",s.pid_Kp);
	usart_sendStr(buf);
	sprintf(buf,"pid_Ki: %d\n\r",s.pid_Ki);
	usart_sendStr(buf);
	sprintf(buf,"pid_Kd: %d\n\r",s.pid_Kd);
	usart_sendStr(buf);
	sprintf(buf,"usart_baud: %d\n\r",s.usart_baud);
	usart_sendStr(buf);


}
