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


/*
 * functionality from sinusoid_drive branch has been merged to master brach 13.2.2017. define SINUSOID_DRIVE to
 * enable sinusoid commutation, comment it out to define trapezoidal commutation.
 *
 */

#define SINUSOID_DRIVE 1

//PID timer period. Counter runs at 1MHz, so f_pid = 1e6/period. 250=4kHz, 125=8Khz, 63=16kHz..
#define PID_TIM_PERIOD 40

//BLDC_CHOPPER_PERIOD defines the PWM clock frequency. Timer runs at 72MHz (Fcpu), so Fpwm = 72e6/period
//4000 = 18kHz
//2000 = 36kHz
#define BLDC_CHOPPER_PERIOD 2000
#ifndef SINUSOID_DRIVE
#define MAX_DUTY BLDC_CHOPPER_PERIOD-50 //100% duty not allowed to allow recharge of high side gate drivers
#else
#define ZERO_DUTY BLDC_CHOPPER_PERIOD/2
#define MAX_DUTY BLDC_CHOPPER_PERIOD/2-50 //100% duty not allowed to allow recharge of high side gate drivers
#endif

#define BLDC_NOL 7//Non-OverLapping, number of clock cycles
#define BLDC_DELAY 100 //Commutation delay. 1= no delay, 2000=7ms.




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
	volatile uint16_t pid_deadband;
	volatile int16_t commutation_offset;

	volatile uint16_t usart_baud; //baud divided by 100 to fit to 16 bits! for example 115200 => 1152

} servoConfig;
void getConfig();

void setConfig(char* param, int16_t value);
void writeConfig();
void printConfiguration();
#endif /* CONFIGURATION_H_ */
