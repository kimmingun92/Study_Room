#ifndef __HW_DRIVER_LED_H_
#define __HW_DRIVER_LED_H_

#include "hw_def.h"
#include "def.h"

typedef struct COLOR{
    RED = 0,
    GREEN,
    BLUE,
    MAGENTA,
    YELLOW
} LED_COLOR

void setLedColor(uint8_t id, LED_COLOR color);
LED_COLOR getLedColor(uint8_t id);

void setLedPower(uint8_t id, uint8_t brightness);
uint8_t getLedPower(uint8_t id);

#endif