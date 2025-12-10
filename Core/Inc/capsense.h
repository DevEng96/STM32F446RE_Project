/**
 * @file    capsense.h
 * @brief
 * @author  Leoni
 * @date    2025-11-30
 */
#ifndef __CAPSENSE_H
#define __CAPSENSE_H

#include "stm32f4xx_hal.h"   // for ADC_HandleTypeDef, HAL_ADC_*
#include <stdint.h>          // for uint16_t, uint8_t


uint16_t Capsense_AdcOnce(void);
uint16_t Capsense_AdcReadAvg(uint8_t samples);
float Capsense_AdcToVolt(uint16_t counts);
float Capsense_GetMoisture();

#endif
