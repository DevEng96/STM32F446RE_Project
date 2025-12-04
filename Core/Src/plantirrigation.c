///*
// * plantirrigation.c
// *
// *  Created on: Nov 30, 2025
// *      Author: leoni
// */
//
//#include "lcd_driver.h"
//#include "main.h"           // for GPIO pins, sTime, sDate, etc.
//#include "digTemp_KY_028.h" // temp sensor
//#include <stdio.h>
//#include "plantirrigation.h"
//#include "capsense.h"
//
//extern RTC_TimeTypeDef sTime;
//extern RTC_DateTypeDef sDate;
//extern uint8_t buttonPressed;   // if it’s global in main.c
//
//
////	printf("\r\nSoil sensor demo starting...\r\n");
////	uint16_t dry_counts = 3000;   // measure in air & set
////	uint16_t wet_counts = 1000;   // measure in water/very wet soil & set
////	lcd_init();
////	lcd_clear();
////	lcd_setLine(127, 0, 127, 31, 1);
////	lcd_setLine(0, 0, 0, 31, 1);
//
//void PlantIrrigation_DebugOutput(void) {
//
//
//	char line1[32];
//	char line2[32];
//
//	snprintf(line1, sizeof(line1), "Temp: %2.1f%cC", tmp, 0x80);
//	snprintf(line2, sizeof(line2), "Moisture: %3.0f%%", pct);
//
//	lcd_clear();
//	lcd_setString(4, 4, line1, LCD_FONT_8, false);
//	lcd_setString(4, 16, line2, LCD_FONT_8, false);
//	lcd_show();
//
//	// manual relay toggle
//	if (HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin)) {
//		HAL_GPIO_TogglePin(RELAIS_K1_GPIO_Port, RELAIS_K1_Pin);
//		buttonPressed = !buttonPressed;
//		HAL_Delay(50);
//	}
//
//	// LED for tank level
//	if (HAL_GPIO_ReadPin(LEVEL_RX_GPIO_Port, LEVEL_RX_Pin)) {
//		setLED(0, 1, 0);
//	} else {
//		setLED(1, 1, 1);
//	}
//
//	printf("Time: %02d:%02d:%02d  Date: %02d.%02d.%04d  "
//			"Moisture=%.1f %%  Temp: %.1f°C  Pump: %d\r\n", sTime.Hours,
//			sTime.Minutes, sTime.Seconds, sDate.Date, sDate.Month,
//			sDate.Year + 2000, pct, tmp, buttonPressed);
//}
//
////		// Map to 0..100% using your own dry/wet calibration points.
////		// Sensor is inverse: wetter -> LOWER counts/voltage.
////
////		if (dry_counts != wet_counts) {
////			// invert so higher % = wetter
////			pct = 100.0f * (float) (dry_counts - raw)
////					/ (float) (dry_counts - wet_counts);
////			if (pct < 0.0f)
////				pct = 0.0f;
////			if (pct > 100.0f)
////				pct = 100.0f;
////		}
////
////		float tmp = readTemp();             // read actual temperature
////
////		char line1[32];
////		char line2[32];
////
////		snprintf(line1, sizeof(line1), "Temp: %2.1f%cC", tmp, 0x80);
////		snprintf(line2, sizeof(line2), "Moisture: %3.0f%%", pct); // rounded to integer %
////		lcd_clear();
////		lcd_setString(4, 4, line1, LCD_FONT_8, false);  // y=4: temperature
////		lcd_setString(4, 16, line2, LCD_FONT_8, false);  // y=16: moisture
////		lcd_show();
////
////		if (HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin)) {
////
////			HAL_GPIO_TogglePin(RELAIS_K1_GPIO_Port, RELAIS_K1_Pin);
////			buttonPressed = !buttonPressed;
////			HAL_Delay(50);
////		}
////		if (HAL_GPIO_ReadPin(LEVEL_RX_GPIO_Port, LEVEL_RX_Pin)) {
////			setLED(0, 1, 0);
////		} else {
////			setLED(1, 1, 1);
////		}
////
////		printf("Time: %02d:%02d:%02d  Date: %02d.%02d.%04d  "
////				"Moisture=%.1f %%  Temp: %.1f°C  Pump: %d\r\n", sTime.Hours,
////				sTime.Minutes, sTime.Seconds, sDate.Date, sDate.Month,
////				sDate.Year + 2000, pct, readTemp(), buttonPressed);
////		HAL_Delay(250);
//
