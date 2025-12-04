/*
 * irrigation_state_machine.c
 *
 *  Created on: Nov 30, 2025
 *      Author: leoni
 */

#include "irrigation_state_machine.h"
#include "logging.h"
#include "main.h"
#include "rtc.h"
#include "capsense.h"
#include "lm75b.h"
#include "lcd_driver.h"
#include "stdio.h"
#include "settings_menu.h"
#include "usart.h"

uint32_t nextCheckTime = 0;
uint32_t pumpStartTime = 0;
uint32_t soakStartTime = 0;
uint16_t pumpCyclesSinceLastSample = 0;
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

bool justEnteredState;
SystemState_t currentState = STATE_IDLE;
float pct = 0.0f;

uint32_t ledBlinkLastToggle = 0;
bool ledBlinkState = false;
static uint8_t pumpCyclesThisCheck = 0;

void Irrigation_Init(void) {
	// reset timers, counters, etc.
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

	switch (currentState) {
	case STATE_IDLE:
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
//		if (HAL_GPIO_ReadPin(BTN_SELECT_GPIO_Port, BTN_SELECT_Pin)
//				== GPIO_PIN_SET) {
//			currentState = STATE_SETTINGS;
//			justEnteredState = 1;
//		}
		break;

	case STATE_CHECK_CONDITIONS:

		float moisture = getMoisture();
		float temp = readTemp();
		bool waterOK = tankLevelOK();
		bool window = inWateringWindow(&sTime);

		if (justEnteredState) {

			logSample(moisture, temp, pumpCyclesSinceLastSample);
			pumpCyclesSinceLastSample = 0;
			pumpCyclesThisCheck = 0;      // <-- reset per 15-min check
			justEnteredState = 0;
			printf("STATE CHECK CONDITIONS [%02u:%02u:%02u]:  %0.2f  %0.2f\r\n",
					sTime.Hours, sTime.Minutes, sTime.Seconds, moisture, temp);

			if (!window) {
				currentState = STATE_IDLE;
				break;
			}
			if (!waterOK) {
				currentState = STATE_ERROR;
				break;
			}
			if (temp < 10.0f) {
				currentState = STATE_IDLE;
				break;
			}
			if (moisture >= moistureMinPct) {
				currentState = STATE_IDLE;
				break;
			}
			// All conditions satisfied → start watering
			currentState = STATE_PUMP_ON;
			justEnteredState = 1;
			break;

			case STATE_PUMP_ON:

			if (pumpCyclesThisCheck >= PUMP_CYCLES_MAX) {
				// safety: make sure pump is OFF
				HAL_GPIO_WritePin(RELAIS_K1_GPIO_Port, RELAIS_K1_Pin,
						GPIO_PIN_RESET);
				currentState = STATE_IDLE;       // or STATE_ERROR if you prefer
				justEnteredState = 1;
				break;
			}

			if (justEnteredState) {
				HAL_GPIO_WritePin(RELAIS_K1_GPIO_Port, RELAIS_K1_Pin,
						GPIO_PIN_SET);
				pumpStartTime = now;
				pumpCyclesSinceLastSample++;
				justEnteredState = 0;
				pumpCyclesThisCheck++;
				printf("STATE PUMP ON:  %u\r\n", pumpCyclesThisCheck);
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
			case STATE_SOAK_WAIT:

			if ((int32_t) (now - soakStartTime) >= (int32_t) SOAK_WAIT_MS) {

				float moisture = getMoisture();
				printf("STATE SOAK WAIT:  %f\r\n", moisture);
				if (moisture
						< moistureMaxPct&& pumpCyclesThisCheck < PUMP_CYCLES_MAX) {
					currentState = STATE_PUMP_ON;
				} else if (pumpCyclesThisCheck >= PUMP_CYCLES_MAX) {
					currentState = STATE_ERROR;
				} else {
					currentState = STATE_CHECK_CONDITIONS;
				}
				justEnteredState = 1;
			}
			break;

			case STATE_SETTINGS:
			{
				if (justEnteredState) {
					Settings_Enter();
					justEnteredState = 0;
				}

				Settings_Tick();

				// Settings_Tick() could set a flag or return a value to indicate "finished"
				if (Settings_IsDone()) {  // or a boolean flag
					Settings_Leave();
					currentState = STATE_IDLE;
					justEnteredState = 1;
				}
				break;
			}

			case STATE_ERROR:
			HAL_GPIO_WritePin(RELAIS_K1_GPIO_Port, RELAIS_K1_Pin,
					GPIO_PIN_RESET);

			if (!tankLevelOK()) {
				blinkLED(LED_BLUE, 500);
			} else if (pumpCyclesThisCheck >= PUMP_CYCLES_MAX) {
				blinkLED(LED_RED, 500);
			}
			if (tankLevelOK()) {
				currentState = STATE_IDLE;
				justEnteredState = 1;
			}
			break;
		}
	}
}

void setLED(int rState, int gState, int bState) {
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, rState);
	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, gState);
	HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, bState);
}

bool inWateringWindow(RTC_TimeTypeDef *t) {
	// Morning: 06:00–08:00  → hours 6 or 7
	if (t->Hours >= morningStartHour && t->Hours < morningEndHour)
		return true;

	// Evening: 20:00–22:00 → hours 20 or 21
	if (t->Hours >= eveningStartHour && t->Hours < eveningEndHour)
		return true;

	return false;
}

bool tankLevelOK(void) {
	return HAL_GPIO_ReadPin(LEVEL_RX_GPIO_Port, LEVEL_RX_Pin);  // HIGH = OK
}

void blinkLED(LedColor_t color, uint32_t intervalMs) {
	uint32_t now = HAL_GetTick();

	// Toggle only when interval elapsed
	if ((now - ledBlinkLastToggle) >= intervalMs) {
		ledBlinkLastToggle = now;
		ledBlinkState = !ledBlinkState;
		switch (color) {
		case LED_RED:
			HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, ledBlinkState);
			break;
		case LED_GREEN:
			HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,
					ledBlinkState);
			break;
		case LED_BLUE:
			HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, ledBlinkState);
			break;
		}
	}
}

