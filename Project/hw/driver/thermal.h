#ifndef __HW_DRIVER_THERMAL_H__
#define __HW_DRIVER_THERMAL_H__

#include "hw_def.h"
#include "def.h"
#include <math.h>
#include <i2c.h>

#define DEV_ADDR (0x33 << 1)
#define TIMEOUT 100

/* MLX90640 Register / Memory Map */
#define MLX90640_RAM_START_ADDR 0x0400
#define MLX90640_RAM_END_ADDR 0x06FF

#define MLX90640_EEPROM_START_ADDR 0x2400
#define MLX90640_EEPROM_END_ADDR 0x273F
#define MLX90640_EEPROM_WORD_COUNT  832

#define MLX90640_FRAME_WORD_COUNT 834

#define MLX90640_STATUS_REG 0x8000
#define MLX90640_NEW_DATA_BIT  0x0008   // bit3만 1 
#define MLX90640_SUBPAGE_BIT 0x0001
#define MLX90640_CONTROL_REG1 0x800D
#define MLX90640_I2C_CONFIG_REG 0x800F

#define MLX90640_PIXEL_COUNT 768

#define MLX90640_CONTROL_REG          0x800D
#define MLX90640_REFRESH_RATE_MASK    0x0380

#define MLX90640_REFRESH_0_5HZ   0
#define MLX90640_REFRESH_1HZ     1
#define MLX90640_REFRESH_2HZ     2
#define MLX90640_REFRESH_4HZ     3
#define MLX90640_REFRESH_8HZ     4
#define MLX90640_REFRESH_16HZ    5
#define MLX90640_REFRESH_32HZ    6
#define MLX90640_REFRESH_64HZ    7

typedef struct
{
    /* VDD 보정 계수 */
    int16_t vdd25;
    int16_t kVdd; // 수정전: float kVdd;

    /* Ta 보정 계수 */
    float kvPTAT;
    float ktPTAT;
    int16_t vPTAT25;
    float alphaPTAT;

    /* Gain */
    int16_t gainEE;

    /* 공통 보정 계수 */
    float tgc;
    float ksTa;

    /* 온도 범위 관련 계수 */
    int16_t ct[4];
    float ksTo[4];

    /* Compensation Pixel 관련 계수 */
    int16_t cpOffset[2];
    float cpAlpha[2];
    float cpKta;
    float cpKv;

    /* 픽셀별 보정 계수 */
    int16_t offset[MLX90640_PIXEL_COUNT];
    float alpha[MLX90640_PIXEL_COUNT];
    float kta[MLX90640_PIXEL_COUNT];
    float kv[MLX90640_PIXEL_COUNT];

    /* 해상도 보정 */
    uint8_t resolutionEE;

} thermal_params_t;

void thermalInit(void);

bool thermalReadRegister(uint16_t reg_addr, uint16_t *data);
bool thermalWriteRegister(uint16_t reg_addr, uint16_t data);

bool thermalReadFrame(uint16_t *frame_buf);
void thermalPrintRawFrame(void);
bool thermalReadCalibData(uint16_t *eeprom_buf);

bool thermalWaitNewData(uint16_t *status);
uint8_t thermalGetSubpage(uint16_t status);
bool thermalClearNewDataFlag(void);
bool thermalReadFrameSynced(uint16_t *frame_buf, uint8_t *subpage);

bool thermalExtractParameters(uint16_t *eeprom_buf, thermal_params_t *params);
float thermalCalculateVdd(uint16_t *frame_buf, thermal_params_t *params);
float thermalCalculateTa(uint16_t *frame_buf, thermal_params_t *params);
bool thermalCalculateTemperature(uint16_t *frame_buf,
                                 thermal_params_t *params,
                                 float emissivity,
                                 float reflected_temp,
                                 float *temperature_buf);

void cliThermal(uint8_t argc, char *argv[]);
void thermalMain(void);

bool thermalSetRefreshRate(uint8_t refresh_rate);

#endif /* __HW_DRIVER_THERMAL_H__ */