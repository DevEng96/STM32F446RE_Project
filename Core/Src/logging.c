/*
 * logging.c
 *
 *  Created on: Nov 30, 2025
 *      Author: leoni
 */

#include "logging.h"
#include <stdio.h>

LogEntry logBuffer[LOG_SAMPLES];
uint16_t logIndex = 0;     // where the next sample will be written
uint16_t logCount = 0;     // how many valid entries we have (<= LOG_SAMPLES)


void logSample(float moisture, float temp, uint16_t pumpCycles) {
	uint32_t now = 0;/* get from RTC, or HAL_GetTick()/1000 */

	logBuffer[logIndex].timestamp = now;
	logBuffer[logIndex].moisture_pct = moisture;
	logBuffer[logIndex].temperature = temp;
	logBuffer[logIndex].pumpCycles = pumpCycles;

	logIndex++;
	if (logIndex >= LOG_SAMPLES) {
		logIndex = 0; // wrap around -> ring buffer
	}

	if (logCount < LOG_SAMPLES) {
		logCount++;
	}
}

void dumpLog(void) {
	uint16_t i;
	// Start from the oldest entry
	uint16_t idx = (logCount < LOG_SAMPLES) ? 0 : logIndex; // when full, logIndex points to oldest

	for (i = 0; i < logCount; i++) {
		LogEntry *e = &logBuffer[idx];

		printf("%lu,%0.1f,%0.1f,%u\r\n", (unsigned long) e->timestamp,
				e->moisture_pct, e->temperature, e->pumpCycles);

		idx++;
		if (idx >= LOG_SAMPLES)
			idx = 0;
	}
}
