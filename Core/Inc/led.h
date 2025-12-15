/**
 * @file    led.h
 * @brief
 * @author  Leoni
 * @date    2025-11-30
 */

#ifndef INC_LED_H_
#define INC_LED_H_

#include "gpio.h"
#include "stdint.h"
#include <stdint.h>
#include <stdbool.h>


typedef enum {
	LED_RED, LED_GREEN, LED_BLUE
} LED_Color_t;

void LED_Set(int rState, int gState, int bState);
void LED_Blink(LED_Color_t color, uint32_t intervalMs);


#endif /* INC_LED_H_ */
