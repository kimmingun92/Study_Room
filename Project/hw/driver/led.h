#ifndef __HW_DRIVER_LED_H_
#define __HW_DRIVER_LED_H_

#include "hw_def.h"
#include "def.h"

enum LED_COLOR{
    RED = 0,
    GREEN,
    BLUE,
    MAGENTA,
    YELLOW
};

void setLedColor(uint8_t id, int color);
int getLedColor(uint8_t id);

void setLedPower(uint8_t id, uint8_t brightness);
uint8_t getLedPower(uint8_t id);

#endif