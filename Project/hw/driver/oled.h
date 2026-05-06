#ifndef __HW_DRIVER_OLED_H__
#define __HW_DRIVER_OLED_H__

#include "hw_def.h"
#include "def.h"

#define OLED_WIDTH        128
#define OLED_HEIGHT       64
#define OLED_ADDR         (0x3C << 1)
#define OLED_TIMEOUT      100

#define OLED_COLOR_BLACK  0
#define OLED_COLOR_WHITE  1

bool oledInit(void);
bool oledWriteCommand(uint8_t cmd);
bool oledWriteData(uint8_t *data, uint16_t size);

void oledClear(void);
void oledUpdate(void);
void oledDrawPixel(uint8_t x, uint8_t y, uint8_t color);
void oledFill(uint8_t color);
void oledDrawBlock(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

void oledDrawChar(uint8_t x, uint8_t y, char c);
void oledDrawString(uint8_t x, uint8_t y, const char *str);
void oledDrawTwoDigitNumber(uint8_t x, uint8_t y, int num);

uint8_t oledTempToLevelAuto(float temp, float min_t, float max_t);
void oledDrawTempBlock(uint8_t x, uint8_t y, uint8_t level);
void oledDrawThermalInfo(float min_t, float avg_t, float max_t);
void oledDrawThermal(float temp[768]);

#endif /* __HW_DRIVER_OLED_H__ */