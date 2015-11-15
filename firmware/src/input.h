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

//!!!! these mean polarity in the STM32 input. board may have inverting opto couplers etc.
#define INPUT_POLARITY 0 // 0=enabled when low, 1 = enabled when high
#define STEP_POLARITY 0 // 0=step on falling edge, 1= step on rising edge
#define DIR_POLARITY 0 //0=CW when 0, 1= CCW when 0.

#define ENABLE_LED_ON GPIO_SetBits(GPIOC, GPIO_Pin_13);
#define ENABLE_LED_OFF GPIO_ResetBits(GPIOC, GPIO_Pin_13);

#define ERROR_LED_ON GPIO_SetBits(GPIOC, GPIO_Pin_14);
#define ERROR_LED_OFF GPIO_ResetBits(GPIOC, GPIO_Pin_14);

volatile uint16_t updateCtr;

volatile uint16_t motor_running;

void initStepDirInput();
void initPWMInput();
void initLeds();

#endif /* INPUT_H_ */
