/*
 * pid.h
 *
 *  Created on: Oct 29, 2015
 *      Author: pekka
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
