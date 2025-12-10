/**
 * @file    settings_menu.c
 * @brief
 * @author  Leoni
 * @date    2025-11-30
 */

#include "settings_menu.h"
#include "lcd_driver.h"
#include <stdio.h>
#include "logging.h"
#include "led.h"

//static uint32_t last_isr_ms = 0;   // debounce time from elsewhere
static uint32_t lastMs = 0;

static Settings_t g_settings = {
    .morningStartHour = 6,
    .morningEndHour   = 9,
    .eveningStartHour = 20,
    .eveningEndHour   = 22,
    .minTempC         = 10.0f,
    .moistureMinPct   = 30.0f,
    .moistureMaxPct   = 50.0f
};


/* Private static state --------------------------------------------------- */

// Menu state
static MenuItem_t   currentMenuItem = MENU_ITEM_WW_MORNING;
static bool         settingsDone    = false;

typedef enum
{
    EDIT_STATE_NONE = 0,
    EDIT_STATE_VALUE1,
    EDIT_STATE_VALUE2
} EditState_t;

static EditState_t  editState       = EDIT_STATE_NONE;

// Button click flags (set in ISR, read & cleared in Settings_Tick)
static volatile uint8_t selectClick = 0;
static volatile uint8_t upClick     = 0;
static volatile uint8_t downClick   = 0;

/* Private static function prototypes ------------------------------------ */

static void     DrawMenu(void);
//static uint8_t  Pressed(GPIO_TypeDef *port, uint16_t pin);
static inline uint8_t Pressed(GPIO_TypeDef *port, uint16_t pin);

static bool     TakeUpClick(void);
static bool     TakeDownClick(void);

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

	LED_Set(0, 1, 0);   // e.g. purple in settings
	DrawMenu();
}

void Settings_Leave(void) {
	lcd_clear();
	lcd_show();
	LED_Set(1, 0, 1); // set green again for main menu
}

bool Settings_IsDone(void) {
	return settingsDone;
}

void Settings_Tick(void) {
	// --- BROWSING MODE: EDIT_STATE_NONE -----------------------------------
	if (editState == EDIT_STATE_NONE) {

		// Move through menu with UP/DOWN
		if (TakeUpClick()) {
			if (currentMenuItem > 0)
				currentMenuItem--;
			else
				currentMenuItem = MENU_ITEM_COUNT - 1;
			DrawMenu();
		} else if (TakeDownClick()) {
			if (currentMenuItem < MENU_ITEM_COUNT - 1)
				currentMenuItem++;
			else
				currentMenuItem = 0;
			DrawMenu();
		}

		// SELECT: either start editing, or if on EXIT, leave settings
		if (Settings_TakeSelectClick()) {
			if (currentMenuItem == MENU_ITEM_EXIT) {
				settingsDone = true;
				return;
			} else if (currentMenuItem == MENU_ITEM_DUMP_LOG) {
				Log_DumpToUart();
				// maybe print “Done.” or flash LED
				return;
			} else {
				editState = EDIT_STATE_VALUE1; // start editing first value
				DrawMenu();     // redraw with edit hint if desired
			}
		}

		return;   // no editing logic in this mode
	}

	// --- EDITING MODE ------------------------------------------------------

	// Common handling: SELECT advances editing state
	if (Settings_TakeSelectClick()) {
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

		DrawMenu();
		return; // we don't change values on the same tick as a select
	}

	// Now handle UP/DOWN edits depending on which item + which editState
	bool changed = false;

	switch (currentMenuItem) {
	case MENU_ITEM_WW_MORNING:
		if (editState == EDIT_STATE_VALUE1) {
			// edit start hour
			if (TakeUpClick()) {
				g_settings.morningStartHour = (g_settings.morningStartHour + 1) % 24;
				changed = true;
			} else if (TakeDownClick()) {
				g_settings.morningStartHour = (g_settings.morningStartHour + 23) % 24;
				changed = true;
			}
		} else if (editState == EDIT_STATE_VALUE2) {
			// edit end hour
			if (TakeUpClick()) {
				g_settings.morningEndHour = (g_settings.morningEndHour + 1) % 24;
				changed = true;
			} else if (TakeDownClick()) {
				g_settings.morningEndHour = (g_settings.morningEndHour + 23) % 24;
				changed = true;
			}
		}
		break;

	case MENU_ITEM_WW_EVENING:
		if (editState == EDIT_STATE_VALUE1) {
			// edit start hour
			if (TakeUpClick()) {
				g_settings.eveningStartHour = (g_settings.eveningStartHour + 1) % 24;
				changed = true;
			} else if (TakeDownClick()) {
				g_settings.eveningStartHour = (g_settings.eveningStartHour + 23) % 24;
				changed = true;
			}
		} else if (editState == EDIT_STATE_VALUE2) {
			// edit end hour
			if (TakeUpClick()) {
				g_settings.eveningEndHour = (g_settings.eveningEndHour + 1) % 24;
				changed = true;
			} else if (TakeDownClick()) {
				g_settings.eveningEndHour = (g_settings.eveningEndHour + 23) % 24;
				changed = true;
			}
		}
		break;

	case MENU_ITEM_MIN_TEMP:
		if (editState == EDIT_STATE_VALUE1) {
			if (TakeUpClick()) {
				g_settings.minTempC += 0.5f;
				changed = true;
			} else if (TakeDownClick()) {
				g_settings.minTempC -= 0.5f;
				changed = true;
			}
		}
		break;

	case MENU_ITEM_MOISTURE_MIN:
		if (editState == EDIT_STATE_VALUE1) {
			if (TakeUpClick()) {
				g_settings.moistureMinPct += 1.0f;
				changed = true;
			} else if (TakeDownClick()) {
				g_settings.moistureMinPct -= 1.0f;
				changed = true;
			}
		}
		break;

	case MENU_ITEM_MOISTURE_MAX:
		if (editState == EDIT_STATE_VALUE1) {
			if (TakeUpClick()) {
				g_settings.moistureMaxPct += 1.0f;
				changed = true;
			} else if (TakeDownClick()) {
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
		DrawMenu();
	}
}

static void DrawMenu(void) {
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

static inline uint8_t Pressed(GPIO_TypeDef *port, uint16_t pin) {
	return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET; // shield pulls low, press = HIGH
}

//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//	uint32_t now = HAL_GetTick();
//	if (now - last_isr_ms < 20) {
//		return;   // simple debounce
//	}
//	last_isr_ms = now;
//
//	if (GPIO_Pin == BTN_SELECT_Pin
//			&& Pressed(BTN_SELECT_GPIO_Port, BTN_SELECT_Pin)) {
//		selectClick = 1;
//	} else if (GPIO_Pin == BTN_UP_Pin
//			&& Pressed(BTN_UP_GPIO_Port, BTN_UP_Pin)) {
//		upClick = 1;
//	} else if (GPIO_Pin == BTN_DOWN_Pin
//			&& Pressed(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin)) {
//		downClick = 1;
//	}
//}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t now = HAL_GetTick();

    // ===== SIMPLE DEBOUNCE =====
    if (now - lastMs < 80) {      // 80ms is safe for mechanical buttons
        return;                   // ignore bounce
    }
    lastMs = now;
    // ===========================

    if (GPIO_Pin == BTN_SELECT_Pin) {
        selectClick = 1;
    }
    else if (GPIO_Pin == BTN_UP_Pin) {
        upClick = 1;
    }
    else if (GPIO_Pin == BTN_DOWN_Pin) {
        downClick = 1;
    }
}

 bool Settings_TakeSelectClick(void) {
	if (selectClick) {
		selectClick = 0;
		return true;
	}
	return false;
}
static bool TakeUpClick(void) {
	if (upClick) {
		upClick = 0;
		return true;
	}
	return false;
}
static bool TakeDownClick(void) {
	if (downClick) {
		downClick = 0;
		return true;
	}
	return false;
}

