/**
 * @file    irrigation_state_machine.c
 * @brief
 * @author  Leoni
 * @date    2025-11-30
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


/* Private static state --------------------------------------------------- */

// High-level system state
static SystemState_t currentState = STATE_IDLE;
static bool justEnteredState = false;
static ErrorCause_t errorCause = ERROR_NONE;

// Time / date
static RTC_TimeTypeDef sTime;
static RTC_DateTypeDef sDate;

// Timing control (using HAL_GetTick)
static uint32_t nextCheckTime = 0;
static uint32_t pumpStartTime = 0;
static uint32_t soakStartTime = 0;

// Pump cycle tracking
static uint16_t pumpCyclesSinceLastSample = 0;
static uint8_t pumpCyclesThisCheck = 0;


/* Private static functions ---------------------------------------------- */

static bool InWateringWindow(RTC_TimeTypeDef *t);
static bool TankLevelOk(void);
static void PrintSystemStatus(void);

void Irrigation_Init(void) {
	lcd_init();
	lcd_clear();
	lcd_show();
	Settings_Init();
	HAL_GPIO_WritePin(LEVEL_TX_GPIO_Port, LEVEL_TX_Pin, 1);
	LED_Set(1, 1, 1);
	justEnteredState = 1;
	nextCheckTime = HAL_GetTick() + CHECK_PERIOD_MS;
	PrintSystemStatus();

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
			LED_Set(1, 0, 1);
			justEnteredState = 0;
		}
		if ((int32_t) (now - nextCheckTime) >= 0) { // signed trick for wraparound
			currentState = STATE_CHECK_CONDITIONS;
			nextCheckTime = now + CHECK_PERIOD_MS;  // schedule next check
			justEnteredState = 1;
		}

		if (Settings_TakeSelectClick()) {
			currentState = STATE_SETTINGS;
			justEnteredState = 1;
		}
		break;
	}

	case STATE_CHECK_CONDITIONS: {

		float moisture = Capsense_GetMoisture();
		float temp = readTemp();
		bool waterOK = TankLevelOk();
		bool window = InWateringWindow(&sTime);

		if (justEnteredState) {

			Log_Sample(moisture, temp, pumpCyclesSinceLastSample);
			pumpCyclesSinceLastSample = 0;
			pumpCyclesThisCheck = 0;
			justEnteredState = 0;
			printf(
					"STATE CHECK CONDITIONS [%02u:%02u:%02u]: Moisture: %0.2f, Temp: %0.2f\r\n",
					sTime.Hours, sTime.Minutes, sTime.Seconds, moisture, temp);
			if (!waterOK) {
				errorCause = ERROR_TANK_EMPTY;
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
			// All conditions satisfied start watering
			currentState = STATE_PUMP_ON;
			justEnteredState = 1;
			break;
		}
	}

	case STATE_PUMP_ON: {

		if (pumpCyclesThisCheck >= PUMP_CYCLES_MAX) {
			HAL_GPIO_WritePin(RELAIS_K1_GPIO_Port, RELAIS_K1_Pin,
					GPIO_PIN_RESET);
			currentState = STATE_ERROR;
			errorCause = ERROR_PUMP_OVERRUN;
			justEnteredState = 1;
			printf("STATE PUMP: Max Pump Cycles Reached\r\n");
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
		break;
	}
	case STATE_SOAK_WAIT: {
		if (justEnteredState) {
			printf("STATE SOAK WAIT\r\n");
			justEnteredState = 0;
		}

		if ((int32_t) (now - soakStartTime) >= (int32_t) SOAK_WAIT_MS) {

			float moisture = Capsense_GetMoisture();
			printf("STATE SOAK WAIT:  %f\r\n", moisture);
			if (TankLevelOk()) {
				currentState = STATE_ERROR;
				errorCause = ERROR_TANK_EMPTY;
			} else if ((moisture < cfg->moistureMaxPct)
					&& (pumpCyclesThisCheck < PUMP_CYCLES_MAX)) {
				currentState = STATE_PUMP_ON;
			} else if (pumpCyclesThisCheck >= PUMP_CYCLES_MAX) {
				currentState = STATE_ERROR;
				errorCause = ERROR_PUMP_OVERRUN;

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
			HAL_GPIO_WritePin(RELAIS_K1_GPIO_Port, RELAIS_K1_Pin,
					GPIO_PIN_RESET);
			LED_Set(1, 1, 1); // all off
		}

		switch (errorCause) {
		case ERROR_TANK_EMPTY:
			LED_Blink(LED_BLUE, 500);
			if (justEnteredState) {
				printf("--Tank Empty\r\n");
				justEnteredState = 0;
			}
			if (TankLevelOk()) {
				errorCause = ERROR_NONE;
				currentState = STATE_IDLE;
				justEnteredState = 1;
			}
			break;

		case ERROR_PUMP_OVERRUN:
			LED_Blink(LED_RED, 500);
			if (justEnteredState) {
				printf("--Pump Overrun\r\n");
				justEnteredState = 0;
			}
			if (Settings_TakeSelectClick()) {
				errorCause = ERROR_NONE;
				pumpCyclesThisCheck = 0;
				currentState = STATE_IDLE;
				justEnteredState = 1;
			}
			float moisture = Capsense_GetMoisture();
			if (moisture > cfg->moistureMaxPct + 5.0f) { // hysteresis
				errorCause = ERROR_NONE;
				currentState = STATE_IDLE;
				justEnteredState = 1;
			}

			break;

		default:
			// No known error cause, just go back to idle
			currentState = STATE_IDLE;
			justEnteredState = 1;
			break;
		}

		break;
	}
	}
}

static bool InWateringWindow(RTC_TimeTypeDef *t) {
	const Settings_t *cfg = Settings_Get();
	if (t->Hours >= cfg->morningStartHour && t->Hours < cfg->morningEndHour)
		return true;

	if (t->Hours >= cfg->eveningStartHour && t->Hours < cfg->eveningEndHour)
		return true;

	return false;
}

static bool TankLevelOk(void) {
	return HAL_GPIO_ReadPin(LEVEL_RX_GPIO_Port, LEVEL_RX_Pin);  // HIGH = OK
}

static void PrintSystemStatus(void) {
	const Settings_t *cfg = Settings_Get();
	RTC_TimeTypeDef t;
	RTC_DateTypeDef d;

	// Read the RTC
	HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);

	printf("\r\n");
	printf("***********************************************\r\n");
	printf("   Tomato Irrigation System Starting Up...\r\n");
	printf("***********************************************\r\n");

	printf("Current RTC Time : %02u:%02u:%02u\r\n", t.Hours, t.Minutes,
			t.Seconds);

	printf("Current RTC Date : %02u.%02u.%02u\r\n", d.Date, d.Month, d.Year);

	printf("\r\n--- SYSTEM SETTINGS --------------\r\n");

	printf("Morning Window    : %02u:00 - %02u:00\r\n", cfg->morningStartHour,
			cfg->morningEndHour);

	printf("Evening Window    : %02u:00 - %02u:00\r\n", cfg->eveningStartHour,
			cfg->eveningEndHour);

	printf("Minimum Temp      : %.1f °C\r\n", cfg->minTempC);

	printf("Moisture Min      : %.1f %%\r\n", cfg->moistureMinPct);
	printf("Moisture Max      : %.1f %%\r\n", cfg->moistureMaxPct);

	printf("-------------------------------\r\n");
	printf("\r\n");
}

