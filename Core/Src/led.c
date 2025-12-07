/*
 * led.c
 *
 *  Created on: Dec 7, 2025
 *      Author: leoni
 */

#include "led.h"

uint32_t ledBlinkLastToggle = 0;
bool ledBlinkState = false;

void setLED(int rState, int gState, int bState) {
	HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, rState);
	HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, gState);
	HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, bState);
}

void blinkLED(LedColor_t color, uint32_t intervalMs) {
	uint32_t now = HAL_GetTick();

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
