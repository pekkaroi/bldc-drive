/*
 * utils.h
 *
 *  Created on: Nov 4, 2015
 *      Author: pekka
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stddef.h>
#include <stdint.h>

#define STM32_CLOCK_HZ 72000000UL
#define STM32_CYCLES_PER_LOOP 6 // This will need tweaking or calculating



void delay_ms(const uint32_t ms);

#endif /* UTILS_H_ */
