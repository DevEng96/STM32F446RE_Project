/*
 * irrigation_state_machine.c
 *
 *  Created on: Nov 30, 2025
 *      Author: leoni
 */

#include "irrigation_state_machine.h"
#include "logging.h"
#include "rtc.h"
#include "capsense.h"
#include "lm75b.h"
#include "lcd_driver.h"
#include "stdio.h"
#include "settings_menu.h"
#include "usart.h"
#include "led.h"

static uint32_t nextCheckTime = 0;
static uint32_t pumpStartTime = 0;
static uint32_t soakStartTime = 0;
static uint16_t pumpCyclesSinceLastSample = 0;
static RTC_TimeTypeDef sTime;
static RTC_DateTypeDef sDate;
static SystemState_t currentState = STATE_IDLE;
static bool justEnteredState = false;
static uint8_t pumpCyclesThisCheck = 0;
static bool selectWasHigh = false;

static bool inWateringWindow(RTC_TimeTypeDef *t);
static bool tankLevelNOK(void);

void Irrigation_Init(void) {
	lcd_init();
	lcd_clear();
	lcd_show();
	Settings_Init();
	HAL_GPIO_WritePin(LEVEL_TX_GPIO_Port, LEVEL_TX_Pin, 1);
	setLED(1, 1, 1);
	justEnteredState = 1;
	nextCheckTime = HAL_GetTick() + CHECK_PERIOD_MS;
}

void Irrigation_Tick(void) {
	uint32_t now = HAL_GetTick();
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	const Settings_t *cfg = Settings_Get();

	switch (currentState) {
	case STATE_IDLE: {
		if (justEnteredState) {
			printf("STATE IDLE\r\n");
			setLED(1, 0, 1);
			justEnteredState = 0;
		}
		if ((int32_t) (now - nextCheckTime) >= 0) { // signed trick for wraparound
			currentState = STATE_CHECK_CONDITIONS;
			nextCheckTime = now + CHECK_PERIOD_MS;  // schedule next check
			justEnteredState = 1;
		}
		bool selNow = (HAL_GPIO_ReadPin(BTN_SELECT_GPIO_Port, BTN_SELECT_Pin)
				== GPIO_PIN_SET);
//		if (HAL_GPIO_ReadPin(BTN_SELECT_GPIO_Port, BTN_SELECT_Pin)
//				== GPIO_PIN_SET) {
//			currentState = STATE_SETTINGS;
//			justEnteredState = 1;
//		}
		if (!selectWasHigh && selNow) {
			currentState = STATE_SETTINGS;
			justEnteredState = 1;
		}

		// remember for next tick
		selectWasHigh = selNow;
		break;
	}

	case STATE_CHECK_CONDITIONS: {

		float moisture = getMoisture();
		float temp = readTemp();
		bool waterNOK = tankLevelNOK();
		bool window = inWateringWindow(&sTime);

		if (justEnteredState) {

			logSample(moisture, temp, pumpCyclesSinceLastSample);
			pumpCyclesSinceLastSample = 0;
			pumpCyclesThisCheck = 0;
			justEnteredState = 0;
			printf(
					"STATE CHECK CONDITIONS [%02u:%02u:%02u]: Moisture: %0.2f, Temp: %0.2f\r\n",
					sTime.Hours, sTime.Minutes, sTime.Seconds, moisture, temp);
			if (waterNOK) {
				currentState = STATE_ERROR;
				justEnteredState = 1;
				break;
			}
			if (!window) {
				currentState = STATE_IDLE;
				justEnteredState = 1;
				break;
			}

			if (temp < cfg->minTempC) {
				currentState = STATE_IDLE;
				justEnteredState = 1;
				break;
			}
			if (moisture >= cfg->moistureMinPct) {
				currentState = STATE_IDLE;
				justEnteredState = 1;
				break;
			}
			// All conditions satisfied → start watering
			currentState = STATE_PUMP_ON;
			justEnteredState = 1;
			break;
		}
	}

	case STATE_PUMP_ON: {

		if (pumpCyclesThisCheck >= PUMP_CYCLES_MAX) {
			// safety: make sure pump is OFF
			HAL_GPIO_WritePin(RELAIS_K1_GPIO_Port, RELAIS_K1_Pin,
					GPIO_PIN_RESET);
			currentState = STATE_IDLE;       // or STATE_ERROR if you prefer
			justEnteredState = 1;
			printf("STATE PUMP: Max Pump Cycles Reached");
			break;
		}

		if (justEnteredState) {
			HAL_GPIO_WritePin(RELAIS_K1_GPIO_Port, RELAIS_K1_Pin, GPIO_PIN_SET);
			pumpStartTime = now;
			pumpCyclesSinceLastSample++;
			justEnteredState = 0;
			pumpCyclesThisCheck++;
			printf("STATE PUMP ON:  %u Cycles\r\n", pumpCyclesThisCheck);
		}
		if ((int32_t) (now - pumpStartTime) >= (int32_t) PUMP_ON_MS) {
			HAL_GPIO_WritePin(RELAIS_K1_GPIO_Port, RELAIS_K1_Pin,
					GPIO_PIN_RESET); // pump OFF
			soakStartTime = now;
			currentState = STATE_SOAK_WAIT;
			justEnteredState = 1;
		}
		//			check max cycle counts
		break;
	}
	case STATE_SOAK_WAIT: {
		if (justEnteredState) {
			printf("STATE SOAK WAIT\r\n");
			justEnteredState = 0;
		}

		if ((int32_t) (now - soakStartTime) >= (int32_t) SOAK_WAIT_MS) {

			float moisture = getMoisture();
			printf("STATE SOAK WAIT:  %f\r\n", moisture);
			if ((moisture < cfg->moistureMaxPct)
					&& (pumpCyclesThisCheck < PUMP_CYCLES_MAX)) {
				currentState = STATE_PUMP_ON;
			} else if (pumpCyclesThisCheck >= PUMP_CYCLES_MAX) {
				currentState = STATE_ERROR;
			} else {
				currentState = STATE_CHECK_CONDITIONS;
			}
			justEnteredState = 1;
		}
		break;
	}

	case STATE_SETTINGS: {

		if (justEnteredState) {
			Settings_Enter();
			printf("STATE SETTINGS\r\n");
			justEnteredState = 0;
		}

		Settings_Tick();

		if (Settings_IsDone()) {
			Settings_Leave();
			currentState = STATE_IDLE;
			justEnteredState = 1;
		}
		break;
	}

	case STATE_ERROR: {
		if (justEnteredState) {
			printf("STATE ERROR\r\n");
			justEnteredState = 0;
			setLED(1, 1, 1); // clear all leds
		}

		HAL_GPIO_WritePin(RELAIS_K1_GPIO_Port, RELAIS_K1_Pin, GPIO_PIN_RESET);

		if (tankLevelNOK()) {
			blinkLED(LED_BLUE, 500);
		} else if (pumpCyclesThisCheck >= PUMP_CYCLES_MAX) {
			blinkLED(LED_RED, 500); // this never really reched / or used, when tanklevel -> ok it will just leave.
		}

		if (!tankLevelNOK()) {
			currentState = STATE_IDLE;
			justEnteredState = 1;
		}
		break;
	}
	}
}

static bool inWateringWindow(RTC_TimeTypeDef *t) {
	const Settings_t *cfg = Settings_Get();
	if (t->Hours >= cfg->morningStartHour && t->Hours < cfg->morningEndHour)
		return true;

	if (t->Hours >= cfg->eveningStartHour && t->Hours < cfg->eveningEndHour)
		return true;

	return false;
}

static bool tankLevelNOK(void) {
	return HAL_GPIO_ReadPin(LEVEL_RX_GPIO_Port, LEVEL_RX_Pin);  // HIGH = OK
}

