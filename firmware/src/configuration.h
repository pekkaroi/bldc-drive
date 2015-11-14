/*
 * configuration.h
 *
 *  Created on: Nov 1, 2015
 *      Author: pekka
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <stdint.h>

#define inputMethod_stepDir 1
#define inputMethod_pwmVelocity 2
#define commutationMethod_HALL 1
#define commutationMethod_Encoder 2

typedef struct  {
	uint16_t inputMethod;
	uint16_t commutationMethod;
	uint16_t encoder_PPR;
	uint16_t encoder_poles;
	int16_t encoder_counts_per_step;
	int16_t pid_Kp;
	int16_t pid_Ki;
	int16_t pid_Kd;

	uint16_t usart_baud; //baud divided by 100 to fit to 16 bits! for example 115200 => 1152

} servoConfig;
void getConfig();

void setConfig(char* param, int16_t value);

void printConfiguration();
#endif /* CONFIGURATION_H_ */
