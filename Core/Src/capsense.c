/**
 * @file    capsense.c
 * @brief
 * @author  Leoni
 * @date    2025-11-30
 */

#include "capsense.h"
#include "stdio.h"


extern ADC_HandleTypeDef hadc1; // use ADC handle from main
static uint16_t dry_counts = 3000;
static uint16_t wet_counts = 1110;

uint16_t Capsense_AdcOnce(void) {
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 5);
	uint16_t v = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return v;
}

uint16_t Capsense_AdcReadAvg(uint8_t samples) {
	uint32_t acc = 0;
	for (uint8_t i = 0; i < samples; i++) {
		acc += Capsense_AdcOnce();
	}
	return (uint16_t) (acc / samples);
}

float Capsense_AdcToVolt(uint16_t counts) {
	return (3.3f * counts) / 4095.0f;
}

float Capsense_GetMoisture() {
	uint16_t raw = Capsense_AdcReadAvg(16);
	float pct = 0.0f;
	if (dry_counts != wet_counts) {
		pct = 100.0f * (float) (dry_counts - raw)
				/ (float) (dry_counts - wet_counts);
		if (pct < 0.0f)
			pct = 0.0f;
		if (pct > 100.0f)
			pct = 100.0f;
	}
	return pct;
}
