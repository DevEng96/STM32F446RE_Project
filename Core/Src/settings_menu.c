/*
 * settings_menu.c
 *
 *  Created on: Nov 30, 2025
 *      Author: leoni
 */

#include "settings_menu.h"
#include "lcd_driver.h"
#include "main.h"
#include <stdio.h>
#include "logging.h"

static uint32_t last_isr_ms = 0;   // debounce time from elsewhere

// ----- Public setting variables (defined here, extern in header) -----
//uint8_t morningStartHour = 6;
//uint8_t morningEndHour = 8;
//uint8_t eveningStartHour = 20;
//uint8_t eveningEndHour = 22;
//float minTempC = 10.0f;
//float moistureMinPct = 25.0f;
//float moistureMaxPct = 40.0f;

static Settings_t g_settings = {
    .morningStartHour = 6,
    .morningEndHour   = 8,
    .eveningStartHour = 20,
    .eveningEndHour   = 22,
    .minTempC         = 10.0f,
    .moistureMinPct   = 25.0f,
    .moistureMaxPct   = 40.0f
};




// ----- Private state -----
static MenuItem_t currentMenuItem = MENU_ITEM_WW_MORNING;
static bool settingsDone = false;

typedef enum {
	EDIT_STATE_NONE, EDIT_STATE_VALUE1, EDIT_STATE_VALUE2
} EditState_t;

static EditState_t editState = EDIT_STATE_NONE;

// button click flags (set in ISR, read+clear in Settings_Tick)
static volatile uint8_t selectClick = 0;
static volatile uint8_t upClick = 0;
static volatile uint8_t downClick = 0;

// ----- Private helper prototypes -----

static void Settings_DrawMenu(void);
static inline uint8_t pressed(GPIO_TypeDef *port, uint16_t pin);

static bool takeSelectClick(void);
static bool takeUpClick(void);
static bool takeDownClick(void);

// LED helper from other module
void setLED(int rState, int gState, int bState);

void Settings_Init(void) {
	// nothing yet
}

const Settings_t* Settings_Get(void)
{
    return &g_settings;
}


void Settings_Enter(void) {
	settingsDone = false;
	currentMenuItem = MENU_ITEM_WW_MORNING;
	editState = EDIT_STATE_NONE;

	// clear any stale button events
	selectClick = 0;
	upClick = 0;
	downClick = 0;

	setLED(0, 1, 0);   // e.g. purple in settings
	Settings_DrawMenu();
}

void Settings_Leave(void) {
	lcd_clear();
	lcd_show();
	setLED(1, 0, 1); // set green again for main menu
}

bool Settings_IsDone(void) {
	return settingsDone;
}

void Settings_Tick(void) {
	// --- BROWSING MODE: EDIT_STATE_NONE -----------------------------------
	if (editState == EDIT_STATE_NONE) {

		// Move through menu with UP/DOWN
		if (takeUpClick()) {
			if (currentMenuItem > 0)
				currentMenuItem--;
			else
				currentMenuItem = MENU_ITEM_COUNT - 1;
			Settings_DrawMenu();
		} else if (takeDownClick()) {
			if (currentMenuItem < MENU_ITEM_COUNT - 1)
				currentMenuItem++;
			else
				currentMenuItem = 0;
			Settings_DrawMenu();
		}

		// SELECT: either start editing, or if on EXIT, leave settings
		if (takeSelectClick()) {
			if (currentMenuItem == MENU_ITEM_EXIT) {
				settingsDone = true;
				return;
			} else if (currentMenuItem == MENU_ITEM_DUMP_LOG) {
				logDumpToUart();
				// maybe print “Done.” or flash LED
				return;
			} else {
				editState = EDIT_STATE_VALUE1; // start editing first value
				Settings_DrawMenu();     // redraw with edit hint if desired
			}
		}

		return;   // no editing logic in this mode
	}

	// --- EDITING MODE ------------------------------------------------------

	// Common handling: SELECT advances editing state
	if (takeSelectClick()) {
		if (editState == EDIT_STATE_VALUE1) {
			// If this item has a second value, go there. Otherwise, back to browsing.
			if (currentMenuItem == MENU_ITEM_WW_MORNING
					|| currentMenuItem == MENU_ITEM_WW_EVENING) {
				editState = EDIT_STATE_VALUE2;
			} else {
				editState = EDIT_STATE_NONE;
			}
		} else if (editState == EDIT_STATE_VALUE2) {
			// after editing second value, back to browsing
			editState = EDIT_STATE_NONE;
		}

		Settings_DrawMenu();
		return; // we don't change values on the same tick as a select
	}

	// Now handle UP/DOWN edits depending on which item + which editState
	bool changed = false;

	switch (currentMenuItem) {
	case MENU_ITEM_WW_MORNING:
		if (editState == EDIT_STATE_VALUE1) {
			// edit start hour
			if (takeUpClick()) {
				g_settings.morningStartHour = (g_settings.morningStartHour + 1) % 24;
				changed = true;
			} else if (takeDownClick()) {
				g_settings.morningStartHour = (g_settings.morningStartHour + 23) % 24;
				changed = true;
			}
		} else if (editState == EDIT_STATE_VALUE2) {
			// edit end hour
			if (takeUpClick()) {
				g_settings.morningEndHour = (g_settings.morningEndHour + 1) % 24;
				changed = true;
			} else if (takeDownClick()) {
				g_settings.morningEndHour = (g_settings.morningEndHour + 23) % 24;
				changed = true;
			}
		}
		break;

	case MENU_ITEM_WW_EVENING:
		if (editState == EDIT_STATE_VALUE1) {
			// edit start hour
			if (takeUpClick()) {
				g_settings.eveningStartHour = (g_settings.eveningStartHour + 1) % 24;
				changed = true;
			} else if (takeDownClick()) {
				g_settings.eveningStartHour = (g_settings.eveningStartHour + 23) % 24;
				changed = true;
			}
		} else if (editState == EDIT_STATE_VALUE2) {
			// edit end hour
			if (takeUpClick()) {
				g_settings.eveningEndHour = (g_settings.eveningEndHour + 1) % 24;
				changed = true;
			} else if (takeDownClick()) {
				g_settings.eveningEndHour = (g_settings.eveningEndHour + 23) % 24;
				changed = true;
			}
		}
		break;

	case MENU_ITEM_MIN_TEMP:
		if (editState == EDIT_STATE_VALUE1) {
			if (takeUpClick()) {
				g_settings.minTempC += 0.5f;
				changed = true;
			} else if (takeDownClick()) {
				g_settings.minTempC -= 0.5f;
				changed = true;
			}
		}
		break;

	case MENU_ITEM_MOISTURE_MIN:
		if (editState == EDIT_STATE_VALUE1) {
			if (takeUpClick()) {
				g_settings.moistureMinPct += 1.0f;
				changed = true;
			} else if (takeDownClick()) {
				g_settings.moistureMinPct -= 1.0f;
				changed = true;
			}
		}
		break;

	case MENU_ITEM_MOISTURE_MAX:
		if (editState == EDIT_STATE_VALUE1) {
			if (takeUpClick()) {
				g_settings.moistureMaxPct += 1.0f;
				changed = true;
			} else if (takeDownClick()) {
				g_settings.moistureMaxPct -= 1.0f;
				changed = true;
			}
		}
		break;

	default:
		// EXIT has no editing; if we ended up here, just go back
		editState = EDIT_STATE_NONE;
		break;
	}

	if (changed) {
		Settings_DrawMenu();
	}
}

static void Settings_DrawMenu(void) {
	lcd_clear();

	char line1[32];
	char line2[32];

	switch (currentMenuItem) {
	case MENU_ITEM_WW_MORNING:
		if (editState == EDIT_STATE_NONE) {
			snprintf(line1, sizeof(line1), "> Morning Window");
		} else if (editState == EDIT_STATE_VALUE1) {
			snprintf(line1, sizeof(line1), "* M Start Hour");
		} else {
			snprintf(line1, sizeof(line1), "* M End Hour");
		}
		snprintf(line2, sizeof(line2), "  %02u:00-%02u:00", g_settings.morningStartHour,
				g_settings.morningEndHour);

		break;

	case MENU_ITEM_WW_EVENING:
		if (editState == EDIT_STATE_NONE) {
			snprintf(line1, sizeof(line1), "> Evening Window");
		} else if (editState == EDIT_STATE_VALUE1) {
			snprintf(line1, sizeof(line1), "* E Start Hour");
		} else {
			snprintf(line1, sizeof(line1), "* E End Hour");
		}

		snprintf(line2, sizeof(line2), "  %02u:00-%02u:00", g_settings.eveningStartHour,
				g_settings.eveningEndHour);
		break;

	case MENU_ITEM_MIN_TEMP:
		if (editState == EDIT_STATE_NONE) {
			snprintf(line1, sizeof(line1), "> Min Temp");
		} else {
			snprintf(line1, sizeof(line1), "* Min Temp");
		}

		snprintf(line2, sizeof(line2), "  %.1f degC", g_settings.minTempC);
		break;

	case MENU_ITEM_MOISTURE_MIN:
		if (editState == EDIT_STATE_NONE) {
			snprintf(line1, sizeof(line1), "> Moisture Min");
		} else {
			snprintf(line1, sizeof(line1), "* Moisture Min");
		}
		snprintf(line2, sizeof(line2), "  %.1f %%", g_settings.moistureMinPct);

		break;

	case MENU_ITEM_MOISTURE_MAX:
		if (editState == EDIT_STATE_NONE) {
			snprintf(line1, sizeof(line1), "> Moisture Max");
		} else {
			snprintf(line1, sizeof(line1), "* Moisture Max");
		}

		snprintf(line2, sizeof(line2), "  %.1f %%", g_settings.moistureMaxPct);
		break;

	case MENU_ITEM_DUMP_LOG:
		snprintf(line1, sizeof(line1), "> Dump Log");
		snprintf(line2, sizeof(line2), "  Press select");
		break;

	case MENU_ITEM_EXIT:
		snprintf(line1, sizeof(line1), "> Exit Settings");
		snprintf(line2, sizeof(line2), "  Press select");
		break;

	default:
		snprintf(line1, sizeof(line1), "Settings");
		snprintf(line2, sizeof(line2), " ");
		break;
	}

	lcd_setString(0, 0, line1, LCD_FONT_8, false);
	lcd_setString(0, 12, line2, LCD_FONT_8, false);
	lcd_show();
}

static inline uint8_t pressed(GPIO_TypeDef *port, uint16_t pin) {
	return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET; // shield pulls low, press = HIGH
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	uint32_t now = HAL_GetTick();
	if (now - last_isr_ms < 20) {
		return;   // simple debounce
	}
	last_isr_ms = now;

	if (GPIO_Pin == BTN_SELECT_Pin
			&& pressed(BTN_SELECT_GPIO_Port, BTN_SELECT_Pin)) {
		selectClick = 1;
	} else if (GPIO_Pin == BTN_UP_Pin
			&& pressed(BTN_UP_GPIO_Port, BTN_UP_Pin)) {
		upClick = 1;
	} else if (GPIO_Pin == BTN_DOWN_Pin
			&& pressed(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin)) {
		downClick = 1;
	}
}

static bool takeSelectClick(void) {
	if (selectClick) {
		selectClick = 0;
		return true;
	}
	return false;
}
static bool takeUpClick(void) {
	if (upClick) {
		upClick = 0;
		return true;
	}
	return false;
}
static bool takeDownClick(void) {
	if (downClick) {
		downClick = 0;
		return true;
	}
	return false;
}

