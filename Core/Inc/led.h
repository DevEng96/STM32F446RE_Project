/*
 * led.h
 *
 *  Created on: Dec 7, 2025
 *      Author: leoni
 */

#ifndef INC_LED_H_
#define INC_LED_H_

#include "gpio.h"
#include "stdint.h"
//#include "led.h"
//#include "main.h"
#include <stdint.h>
#include <stdbool.h>


typedef enum {
	LED_RED, LED_GREEN, LED_BLUE
} LedColor_t;

void setLED(int rState, int gState, int bState);
void blinkLED(LedColor_t color, uint32_t intervalMs);
#endif /* INC_LED_H_ */
