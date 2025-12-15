/**
 * @file    logging.h
 * @brief
 * @author  Leoni
 * @date    2025-11-30
 */

#ifndef INC_LOGGING_H_
#define INC_LOGGING_H_
#include <stdint.h>

#define LOG_SAMPLES   (4 * 24 * 7)   // 672
typedef struct {
	uint32_t timestamp;
	float moisturePct;
	float temperature;
	uint16_t pumpCycles;
	uint16_t year;
	uint16_t month;
	uint16_t day;
	uint16_t hour;
	uint16_t minute;
} LogEntry;

void Log_Sample(float moisture, float temp, uint16_t pumpCycles);
void Log_DumpToUart();

#endif /* INC_LOGGING_H_ */
