/*
 * usart.h
 *
 *  Created on: Oct 28, 2015
 *      Author: pekka
 */

#ifndef USART_H_
#define USART_H_

#include "configuration.h"

#define USART USART3
void initUSART(uint16_t baud);
void usart_sendChar(char chr);
void usart_sendStr(char *str);

extern volatile servoConfig s;

#endif /* USART_H_ */
