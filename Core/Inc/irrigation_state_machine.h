/*
 * irrigation_state_machine.h
 *
 *  Created on: Nov 30, 2025
 *      Author: leoni
 */

#ifndef INC_IRRIGATION_STATE_MACHINE_H_
#define INC_IRRIGATION_STATE_MACHINE_H_

#include <stdint.h>
#include <stdbool.h>
#include "rtc.h"

typedef enum {
	STATE_IDLE,
	STATE_CHECK_CONDITIONS,
	STATE_PUMP_ON,
	STATE_SOAK_WAIT,
	STATE_ERROR,
	STATE_SETTINGS
} SystemState_t;

typedef enum {
	LED_RED, LED_GREEN, LED_BLUE
} LedColor_t;

//#define CHECK_PERIOD_MS   (10UL * 60UL * 1000UL)  // 15 minutes
//#define PUMP_ON_MS       (10UL*1000UL)   // 10 seconds example
//#define PUMP_CYCLES_MAX	5
//#define SOAK_WAIT_MS     (10UL*60UL*1000UL) // 10 minutes example

#define CHECK_PERIOD_MS   (15UL * 1000UL)  // 15 minutes
#define PUMP_ON_MS       (5UL*1000UL)   // 10 seconds example
#define PUMP_CYCLES_MAX	5
#define SOAK_WAIT_MS     (30UL*1000UL) // 10 minutes example

void Irrigation_Init(void);
void Irrigation_Tick(void);
bool inWateringWindow(RTC_TimeTypeDef *t);
bool tankLevelOK(void);
void setLED(int rState, int gState, int bState);
void blinkLED(LedColor_t color, uint32_t intervalMs);

#endif /* INC_IRRIGATION_STATE_MACHINE_H_ */
