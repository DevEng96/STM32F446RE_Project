#ifndef __CAPSENSE_H
#define __CAPSENSE_H

#include "stm32f4xx_hal.h"   // for ADC_HandleTypeDef, HAL_ADC_*
#include <stdint.h>          // for uint16_t, uint8_t


uint16_t read_adc_once(void);
uint16_t read_adc_avg(uint8_t samples);
float adc_counts_to_volt(uint16_t counts);
float getMoisture();

#endif
