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


#ifndef INPUT_H_
#define INPUT_H_
#include <stdint.h>

extern volatile uint8_t dir;

#define POWER_LED_ON GPIO_SetBits(GPIOC, GPIO_Pin_15);
#define POWER_LED_OFF GPIO_ResetBits(GPIOC, GPIO_Pin_15);

#define ENABLE_LED_ON GPIO_SetBits(GPIOC, GPIO_Pin_13);
#define ENABLE_LED_OFF GPIO_ResetBits(GPIOC, GPIO_Pin_13);

#define ERROR_LED_ON GPIO_SetBits(GPIOC, GPIO_Pin_14);
#define ERROR_LED_OFF GPIO_ResetBits(GPIOC, GPIO_Pin_14);

extern volatile uint16_t updateCtr;

extern volatile uint16_t motor_running;

void initStepDirInput();
void initPWMInput();
void initLeds();

#endif /* INPUT_H_ */
