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

#ifndef ENCODER_H_
#define ENCODER_H_
#include "configuration.h"
#define ENCODER_TIM TIM2
//#define ENCODER_TIM2_REMAP //with this TIM2 connected to PA15 and PB3

void initEncoder();
void getEncoderCount();


uint16_t findCurrentEncoderCommutationPos();
uint16_t findNextEncoderCommutationPos(int8_t dir);


extern volatile servoConfig s;

int32_t encoder_count;

uint16_t encoder_shaft_pos; //this is the shaft position as encoder counts
uint16_t encoder_full_rounds;
uint16_t encoder_commutation_pos; //this is shaft position from the beginning of current commuatiton sequence.

uint8_t encoder_commutation_table[4096]; //20 poles max //8096 PPR max at the moment.

uint16_t encoder_next_commutation_cnt_cw;
uint16_t encoder_next_commutation_cnt_ccw;




#endif /* ENCODER_H_ */
