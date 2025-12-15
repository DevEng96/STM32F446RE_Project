/**
 * @file    capsense.h
 * @brief
 * @author  Leoni
 * @date    2025-11-30
 */
#ifndef __CAPSENSE_H
#define __CAPSENSE_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

uint16_t Capsense_AdcOnce(void);
uint16_t Capsense_AdcReadAvg(uint8_t samples);
float Capsense_AdcToVolt(uint16_t counts);
float Capsense_GetMoisture();

#endif
