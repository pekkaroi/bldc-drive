/*
 	bldc-drive Cheap and simple brushless DC motor driver designed for CNC applications using STM32 microcontroller.
	Copyright (C) 2015 Pekka Roivainen

	Pid loop is based on F103ServoDrive project of Mihai.
	http://www.cnczone.com/forums/open-source-controller-boards/283428-cnc.html

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

#ifndef PID_H_
#define PID_H_

#include "configuration.h"
#include <stdint.h>
void initPid();
void updatePid();

extern volatile servoConfig s;

int32_t pid_requested_position;
uint32_t pid_max_pos_error;
int32_t pid_integrated_error;
int32_t pid_prev_position_error;


#endif /* PID_H_ */
