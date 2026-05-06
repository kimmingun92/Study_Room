#include "oled.h"

static uint8_t oled_buf[OLED_WIDTH * OLED_HEIGHT / 8];

static const uint8_t font5x7[][5] =
{
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x36, 0x36, 0x00, 0x00},
    {0x3E, 0x51, 0x49, 0x45, 0x3E},
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46},
    {0x21, 0x41, 0x45, 0x4B, 0x31},
    {0x18, 0x14, 0x12, 0x7F, 0x10},
    {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3C, 0x4A, 0x49, 0x49, 0x30},
    {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36},
    {0x06, 0x49, 0x49, 0x29, 0x1E},
    {0x7E, 0x11, 0x11, 0x11, 0x7E},
    {0x3E, 0x41, 0x49, 0x49, 0x7A},
    {0x00, 0x41, 0x7F, 0x41, 0x00},
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    {0x7F, 0x04, 0x08, 0x10, 0x7F},
    {0x1F, 0x20, 0x40, 0x20, 0x1F},
    {0x63, 0x14, 0x08, 0x14, 0x63},
};

static uint8_t oledGetFontIndex(char c)
{
    if (c == ' ') return 0;
    if (c == ':') return 1;

    if (c >= '0' && c <= '9')
    {
        return 2 + (c - '0');
    }

    if (c == 'A') return 12;
    if (c == 'G') return 13;
    if (c == 'I') return 14;
    if (c == 'M') return 15;
    if (c == 'N') return 16;
    if (c == 'V') return 17;
    if (c == 'X') return 18;

    return 0;
}

bool oledInit(void)
{
    if (HAL_I2C_IsDeviceReady(&hi2c1, OLED_ADDR, 3, OLED_TIMEOUT) != HAL_OK)
    {
        return false;
    }

    oledWriteCommand(0xAE);

    oledWriteCommand(0x20);
    oledWriteCommand(0x00);

    oledWriteCommand(0xB0);

    oledWriteCommand(0xC8);
    oledWriteCommand(0x00);
    oledWriteCommand(0x10);
    oledWriteCommand(0x40);

    oledWriteCommand(0x81);
    oledWriteCommand(0x7F);

    oledWriteCommand(0xA1);
    oledWriteCommand(0xA6);

    oledWriteCommand(0xA8);
    oledWriteCommand(0x3F);

    oledWriteCommand(0xD3);
    oledWriteCommand(0x00);

    oledWriteCommand(0xD5);
    oledWriteCommand(0x80);

    oledWriteCommand(0xD9);
    oledWriteCommand(0xF1);

    oledWriteCommand(0xDA);
    oledWriteCommand(0x12);

    oledWriteCommand(0xDB);
    oledWriteCommand(0x40);

    oledWriteCommand(0x8D);
    oledWriteCommand(0x14);

    oledClear();
    oledUpdate();

    oledWriteCommand(0xAF);

    return true;
}

bool oledWriteCommand(uint8_t cmd)
{
    uint8_t buf[2];

    buf[0] = 0x00;
    buf[1] = cmd;

    if (HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buf, 2, OLED_TIMEOUT) == HAL_OK)
    {
        return true;
    }

    return false;
}

bool oledWriteData(uint8_t *data, uint16_t size)
{
    uint8_t buf[129];

    if (size > 128)
    {
        return false;
    }

    buf[0] = 0x40;
    memcpy(&buf[1], data, size);

    if (HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buf, size + 1, OLED_TIMEOUT) == HAL_OK)
    {
        return true;
    }

    return false;
}

void oledClear(void)
{
    memset(oled_buf, 0x00, sizeof(oled_buf));
}

void oledUpdate(void)
{
    for (uint8_t page = 0; page < 8; page++)
    {
        oledWriteCommand(0xB0 + page);
        oledWriteCommand(0x00);
        oledWriteCommand(0x10);

        oledWriteData(&oled_buf[OLED_WIDTH * page], OLED_WIDTH);
    }
}

void oledDrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
    {
        return;
    }

    uint8_t page = y / 8;
    uint16_t index = x + page * OLED_WIDTH;
    uint8_t bit = y % 8;

    if (color == OLED_COLOR_WHITE)
    {
        oled_buf[index] |= (1 << bit);
    }
    else
    {
        oled_buf[index] &= ~(1 << bit);
    }
}

void oledFill(uint8_t color)
{
    if (color == OLED_COLOR_WHITE)
    {
        memset(oled_buf, 0xFF, sizeof(oled_buf));
    }
    else
    {
        memset(oled_buf, 0x00, sizeof(oled_buf));
    }
}

void oledDrawBlock(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
    for (uint8_t yy = 0; yy < h; yy++)
    {
        for (uint8_t xx = 0; xx < w; xx++)
        {
            oledDrawPixel(x + xx, y + yy, color);
        }
    }
}

void oledDrawChar(uint8_t x, uint8_t y, char c)
{
    uint8_t font_index = oledGetFontIndex(c);

    for (uint8_t col = 0; col < 5; col++)
    {
        uint8_t line = font5x7[font_index][col];

        for (uint8_t row = 0; row < 7; row++)
        {
            if (line & (1 << row))
            {
                oledDrawPixel(x + col, y + row, OLED_COLOR_WHITE);
            }
            else
            {
                oledDrawPixel(x + col, y + row, OLED_COLOR_BLACK);
            }
        }
    }

    for (uint8_t row = 0; row < 7; row++)
    {
        oledDrawPixel(x + 5, y + row, OLED_COLOR_BLACK);
    }
}

void oledDrawString(uint8_t x, uint8_t y, const char *str)
{
    while (*str)
    {
        oledDrawChar(x, y, *str);
        x += 6;
        str++;

        if (x > OLED_WIDTH - 6)
        {
            break;
        }
    }
}

void oledDrawTwoDigitNumber(uint8_t x, uint8_t y, int num)
{
    if (num < 0)
    {
        num = 0;
    }

    if (num > 99)
    {
        num = 99;
    }

    oledDrawChar(x, y, '0' + (num / 10));
    oledDrawChar(x + 6, y, '0' + (num % 10));
}

uint8_t oledTempToLevelAuto(float temp, float min_t, float max_t)
{
    if (max_t <= min_t)
    {
        return 0;
    }

    float norm = (temp - min_t) / (max_t - min_t);

    if (norm < 0.0f)
    {
        norm = 0.0f;
    }

    if (norm > 1.0f)
    {
        norm = 1.0f;
    }

    uint8_t level = (uint8_t)(norm * 8.0f);

    if (level > 8)
    {
        level = 8;
    }

    return level;
}

void oledDrawTempBlock(uint8_t x, uint8_t y, uint8_t level)
{
    oledDrawBlock(x, y, 4, 2, OLED_COLOR_BLACK);

    for (uint8_t i = 0; i < level; i++)
    {
        uint8_t px = i % 4;
        uint8_t py = i / 4;

        oledDrawPixel(x + px, y + py, OLED_COLOR_WHITE);
    }
}

void oledDrawThermalInfo(float min_t, float avg_t, float max_t)
{
    int min_i = (int)(min_t + 0.5f);
    int avg_i = (int)(avg_t + 0.5f);
    int max_i = (int)(max_t + 0.5f);

    oledDrawBlock(0, 48, 128, 16, OLED_COLOR_BLACK);

    oledDrawString(0, 49, "MIN:");
    oledDrawTwoDigitNumber(24, 49, min_i);

    oledDrawString(42, 49, "AVG:");
    oledDrawTwoDigitNumber(66, 49, avg_i);

    oledDrawString(84, 49, "MAX:");
    oledDrawTwoDigitNumber(108, 49, max_i);
}

void oledDrawThermal(float temp[768])
{
    float min_t = temp[0];
    float max_t = temp[0];
    float sum_t = temp[0];

    for (int i = 1; i < 768; i++)
    {
        if (temp[i] < min_t)
        {
            min_t = temp[i];
        }

        if (temp[i] > max_t)
        {
            max_t = temp[i];
        }

        sum_t += temp[i];
    }

    float avg_t = sum_t / 768.0f;

    oledClear();

    float display_min = avg_t - 0.2f;
    float display_max = avg_t + 1.5f;

    if (display_max > max_t)
    {
        display_max = max_t;
    }

    if ((display_max - display_min) < 0.8f)
    {
        display_max = display_min + 0.8f;
    }

    for (uint8_t y = 0; y < 24; y++)
    {
        for (uint8_t x = 0; x < 32; x++)
        {
            float t = temp[y * 32 + x];

            uint8_t level = oledTempToLevelAuto(t, display_min, display_max);

            if (level <= 1)
            {
                level = 0;
            }

            oledDrawTempBlock(x * 4, y * 2, level);
        }
    }

    oledDrawThermalInfo(min_t, avg_t, max_t);

    oledUpdate();
}