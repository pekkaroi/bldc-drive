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

#include "utils.h"
#include <stddef.h>
#include <stdint.h>

void delay_ms(const uint32_t ms)
{

    uint32_t ms2 = ms*STM32_CLOCK_HZ / 1000 / STM32_CYCLES_PER_LOOP;

    asm volatile(" mov r0, %[ms2] \n\t"
             "1: subs r0, #1 \n\t"
             " bhi 1b \n\t"
             :
             : [ms2] "r" (ms2)
             : "r0");
}
