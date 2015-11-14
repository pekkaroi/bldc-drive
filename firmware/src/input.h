/*
 * input.h
 *
 *  Created on: Oct 26, 2015
 *      Author: pekka
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

#endif /* INPUT_H_ */
