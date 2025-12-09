/*
 * settings_menu.h
 */

#ifndef INC_SETTINGS_MENU_H_
#define INC_SETTINGS_MENU_H_

#include <stdint.h>
#include <stdbool.h>

// Menu items (only needed if something outside wants to know which item is selected)
typedef enum {
    MENU_ITEM_WW_MORNING = 0,
    MENU_ITEM_WW_EVENING,
    MENU_ITEM_MIN_TEMP,
    MENU_ITEM_MOISTURE_MIN,
    MENU_ITEM_MOISTURE_MAX,
	MENU_ITEM_DUMP_LOG,
    MENU_ITEM_EXIT,
    MENU_ITEM_COUNT
} MenuItem_t;

typedef struct {
    uint8_t morningStartHour;
    uint8_t morningEndHour;
    uint8_t eveningStartHour;
    uint8_t eveningEndHour;
    float   minTempC;
    float   moistureMinPct;
    float   moistureMaxPct;
} Settings_t;

// Public API of the settings module
void Settings_Init(void);
void Settings_Enter(void);
void Settings_Leave(void);
void Settings_Tick(void);
bool Settings_IsDone(void);

const Settings_t* Settings_Get(void);
bool settings_takeSelectClick(void);

// Settings values used by the irrigation logic
//extern uint8_t morningStartHour;
//extern uint8_t morningEndHour;
//extern uint8_t eveningStartHour;
//extern uint8_t eveningEndHour;
//extern float   minTempC;
//extern float   moistureMinPct;
//extern float   moistureMaxPct;



#endif /* INC_SETTINGS_MENU_H_ */
