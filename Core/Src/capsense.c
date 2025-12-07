#include "capsense.h"


extern ADC_HandleTypeDef hadc1; // use the ADC handle from main


static uint16_t dry_counts = 3000;   // measure in air & set
static uint16_t wet_counts = 1000;   // measure in water/very wet soil & set
// if i want to change them later from elsewhere:
//void Capsense_SetCalibration(uint16_t dry, uint16_t wet);
//void Capsense_GetCalibration(uint16_t *dry, uint16_t *wet);

uint16_t read_adc_once(void) {
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 5);
	uint16_t v = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return v;
}

uint16_t read_adc_avg(uint8_t samples) {
	uint32_t acc = 0;
	for (uint8_t i = 0; i < samples; i++) {
		acc += read_adc_once();
	}
	return (uint16_t) (acc / samples);
}

float adc_counts_to_volt(uint16_t counts) {
	return (3.3f * counts) / 4095.0f;
}

float getMoisture() {
	uint16_t raw = read_adc_avg(16);
//	uint16_t raw = read_adc_avg(16);
	//	float v = adc_counts_to_volt(raw);
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
