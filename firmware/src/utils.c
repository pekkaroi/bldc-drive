/*
 * utils.c
 *
 *  Created on: Nov 4, 2015
 *      Author: pekka
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
