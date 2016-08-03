/*
 	bldc-drive Cheap and simple brushless DC motor driver designed for CNC applications using STM32 microcontroller.

    The systick based delay_ms was learned from http://www.micromouseonline.com/2016/02/02/systick-configuration-made-easy-on-the-stm32/


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
#include <stm32f10x_rcc.h>

void systickInit (uint16_t frequency)
{
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq (&RCC_Clocks);
  (void) SysTick_Config (RCC_Clocks.HCLK_Frequency / frequency);
}
static volatile uint32_t ticks;

void SysTick_Handler (void)
{
  ticks++;
}

uint32_t millis (void)
{
  return ticks;
}

void delay_ms (uint32_t t)
{
  uint32_t start, end;
  start = millis();
  end = start + t;
  if (start < end) { while ( (millis() >= start) && (millis() < end)) {
    };
  }
}
/*
void delay_ms(const uint32_t ms)
{

    uint32_t ms2 = ms*STM32_CLOCK_HZ / 1000 / STM32_CYCLES_PER_LOOP;

    asm volatile(" mov r0, %[ms2] \n\t"
             "loop: subs r0, #1 \n\t"
             " bhi loop \n\t"
             :
             : [ms2] "r" (ms2)
             : "r0");
}
*/
