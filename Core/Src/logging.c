/*
 * logging.c
 *
 *  Created on: Nov 30, 2025
 *      Author: leoni
 */

#include "logging.h"
#include <stdio.h>
#include "rtc.h"

static LogEntry  logBuffer[LOG_SAMPLES];
//LogEntry logBuffer[LOG_SAMPLES];
static uint16_t logIndex = 0;     // where the next sample will be written
static uint16_t logCount = 0;     // how many valid entries we have (<= LOG_SAMPLES)

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

void logDumpToUart() {

	printf("\r\n--- LOG DUMP START ---\r\n");
	for (uint16_t i = 0; i < logCount; i++) {
		LogEntry *e = &logBuffer[i];
		printf("%04u-%02u-%02u %02u:%02u, %.1f%%, %.1fC, %u cycles\r\n",
				e->year, e->month, e->day, e->hour, e->minute, e->moisture_pct,
				e->temperature, e->pumpCycles);
	}
	printf("--- LOG DUMP END ---\r\n\r\n");

}

//void dumpLog(void) {
//	uint16_t i;
//	uint16_t idx = (logCount < LOG_SAMPLES) ? 0 : logIndex; // when full, logIndex points to oldest
//
//	for (i = 0; i < logCount; i++) {
//		LogEntry *e = &logBuffer[idx];
//
//		printf("%lu,%0.1f,%0.1f,%u\r\n", (unsigned long) e->timestamp,
//				e->moisture_pct, e->temperature, e->pumpCycles);
//
//		idx++;
//		if (idx >= LOG_SAMPLES)
//			idx = 0;
//	}
//}
