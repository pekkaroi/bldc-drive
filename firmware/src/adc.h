/*
 * adc.h
 *
 *  Created on: Oct 26, 2015
 *      Author: pekka
 */

#ifndef ADC_H_
#define ADC_H_

#define ADC_TIM TIM5
#define ADC_TIM_PERIPH RCC_APB1Periph_TIM5
volatile uint16_t ADC_value;
void initADC();


#endif /* ADC_H_ */
