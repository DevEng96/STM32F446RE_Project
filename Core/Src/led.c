/**
 * @file    led.c
 * @brief
 * @author  Leoni
 * @date    2025-11-30
 */

#include "led.h"

static uint32_t ledBlinkLastToggle = 0;
static bool     ledBlinkState      = false;




void LED_Set(int rState, int gState, int bState) {
    HAL_GPIO_WritePin(LED_RED_GPIO_Port,   LED_RED_Pin,   rState ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, gState ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port,  LED_BLUE_Pin,  bState ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LED_Blink(LED_Color_t color, uint32_t intervalMs) {
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
