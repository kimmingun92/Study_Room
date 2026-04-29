#ifndef __HW_DRIVER_THERMAL_H__
#define __HW_DRIVER_THERMAL_H__

#include "hw_def.h"
#include "def.h"

#define DEV_ADDR (0x33 << 1)

/* MLX90640 Register / Memory Map */
#define MLX90640_RAM_START_ADDR 0x0400
#define MLX90640_RAM_END_ADDR 0x06FF
#define MLX90640_EEPROM_START_ADDR 0x2400
#define MLX90640_EEPROM_END_ADDR 0x273F

#define MLX90640_STATUS_REG 0x8000
#define MLX90640_CONTROL_REG1 0x800D
#define MLX90640_I2C_CONFIG_REG 0x800F

#define MLX90640_PIXEL_COUNT 768

#define TIMEOUT 100

void thermalInit(void);

bool thermalReadRegister(uint16_t reg_addr, uint16_t *data);
bool thermalWriteRegister(uint16_t reg_addr, uint16_t data);

bool thermalReadFrame(uint16_t *frame_buf);
void thermalPrintRawFrame(void);

#endif /* __HW_DRIVER_THERMAL_H__ */