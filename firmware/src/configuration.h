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


#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <stdint.h>

#define inputMethod_stepDir 1
#define inputMethod_pwmVelocity 2
#define commutationMethod_HALL 1
#define commutationMethod_Encoder 2

#define is_ena_inverted (s.invert_dirstepena>>0)&1
#define is_step_inverted (s.invert_dirstepena>>1)&1
#define is_dir_inverted (s.invert_dirstepena>>2)&1

typedef struct  {
	volatile uint16_t inputMethod;
	volatile uint16_t commutationMethod;
	volatile uint16_t encoder_PPR;
	volatile uint16_t encoder_poles;
	volatile uint16_t max_error;
	volatile uint16_t invert_dirstepena;
	volatile int16_t encoder_counts_per_step;
	volatile uint16_t max_current;
	volatile int16_t pid_Kp;
	volatile int16_t pid_Ki;
	volatile int16_t pid_Kd;
	volatile int16_t pid_FF1;
	volatile int16_t pid_FF2;

	volatile uint16_t usart_baud; //baud divided by 100 to fit to 16 bits! for example 115200 => 1152

} servoConfig;
void getConfig();

void setConfig(char* param, int16_t value);
void writeConfig();
void printConfiguration();
#endif /* CONFIGURATION_H_ */
