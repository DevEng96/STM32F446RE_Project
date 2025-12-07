/*
 * logging.h
 *
 *  Created on: Nov 30, 2025
 *      Author: leoni
 */

#ifndef INC_LOGGING_H_
#define INC_LOGGING_H_
#include <stdint.h>

#define LOG_SAMPLES   (4 * 24 * 7)   // 672
typedef struct {
	uint32_t timestamp;   // e.g. UNIX time or minutes since start
	float moisture_pct;   // 0..100%
	float temperature;    // °C
	uint16_t pumpCycles;  // cycles since last sample, or cumulative
	uint16_t year;
	uint16_t month;
	uint16_t day;
	uint16_t hour;
	uint16_t minute;
} LogEntry;

void logSample(float moisture, float temp, uint16_t pumpCycles);
void logDumpToUart();

#endif /* INC_LOGGING_H_ */
