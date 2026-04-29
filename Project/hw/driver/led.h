#ifndef __HW_DRIVER_LED_H_
#define __HW_DRIVER_LED_H_

#include "hw_def.h"
#include "def.h"

typedef enum {
    LED_OFF,
    LED_YELLOW,
    LED_WHITE,
    LED_WARM_WHITE
} LED_COLOR;

void ledInit(void);
void ledUpdate(void);
void ledCommandProcess(const char *cmd);

void setLedColor(uint8_t id, LED_COLOR color);
LED_COLOR getLedColor(uint8_t id);
void setLedPower(uint8_t id, uint8_t brightness);
uint8_t getLedPower(uint8_t id);

#endif
