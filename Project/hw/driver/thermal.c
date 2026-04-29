#include "thermal.h"
//#include <i2c.h>


void thermalInit(void)
{
    if(HAL_I2C_IsDeviceReady(&hi2c1, DEV_ADDR, 3, TIMEOUT) == HAL_OK)
    {
        uartPrintf(0, "Temperature measure start!!\r\n");
    }
    else 
    {
        uartPrintf(0, "Error!!\r\n");
    }
}

// 
bool thermalReadRegister(uint16_t reg_addr, uint16_t *data)
{
    uint8_t buf[2]; 
}

bool thermalWriteRegister(uint16_t reg_addr, uint16_t data)
{

}

//MLX90640의 픽셀 RAM 데이터를 읽는 함수
bool thermalReadFrame(uint16_t *frame_buf)
{

}

void thermalPrintRawFrame(void)
{

}

/*
1단계: MLX90640 I2C 연결 확인
2단계: Status Register 읽기
3단계: RAM raw data 읽기
4단계: UART로 raw data 출력
5단계: 보정 계산해서 실제 온도 °C 변환
*/

#if 0
#include "thermal.h"
#include "i2c.h"
#include "uart.h"
#include "main.h"
#include <stdio.h>

void thermalInit(void)
{
    uint16_t status = 0;

    if (thermalReadRegister(MLX90640_STATUS_REG, &status) == true)
    {
        uartPrintf(0, "MLX90640 Ready\r\n");
        uartPrintf(0, "Status Reg: 0x%04X\r\n", status);
    }
    else
    {
        uartPrintf(0, "MLX90640 Not Found\r\n");
    }
}

bool thermalReadRegister(uint16_t reg_addr, uint16_t *data)
{
    uint8_t buf[2];

    if (HAL_I2C_Mem_Read(&hi2c1,
                         MLX90640_ADDR,
                         reg_addr,
                         I2C_MEMADD_SIZE_16BIT,
                         buf,
                         2,
                         100) != HAL_OK)
    {
        return false;
    }

    *data = ((uint16_t)buf[0] << 8) | buf[1];

    return true;
}

bool thermalWriteRegister(uint16_t reg_addr, uint16_t data)
{
    uint8_t buf[2];

    buf[0] = (data >> 8) & 0xFF;
    buf[1] = data & 0xFF;

    if (HAL_I2C_Mem_Write(&hi2c1,
                          MLX90640_ADDR,
                          reg_addr,
                          I2C_MEMADD_SIZE_16BIT,
                          buf,
                          2,
                          100) != HAL_OK)
    {
        return false;
    }

    HAL_Delay(5);

    return true;
}

bool thermalReadFrame(uint16_t *frame_buf)
{
    uint8_t raw_buf[MLX90640_PIXEL_COUNT * 2];

    if (HAL_I2C_Mem_Read(&hi2c1,
                         MLX90640_ADDR,
                         MLX90640_RAM_START_ADDR,
                         I2C_MEMADD_SIZE_16BIT,
                         raw_buf,
                         MLX90640_PIXEL_COUNT * 2,
                         1000) != HAL_OK)
    {
        return false;
    }

    for (int i = 0; i < MLX90640_PIXEL_COUNT; i++)
    {
        frame_buf[i] = ((uint16_t)raw_buf[i * 2] << 8) | raw_buf[i * 2 + 1];
    }

    return true;
}

void thermalPrintRawFrame(void)
{
    uint16_t frame[MLX90640_PIXEL_COUNT];

    if (thermalReadFrame(frame) == false)
    {
        uartPrintf(0, "MLX90640 frame read failed\r\n");
        return;
    }

    uartPrintf(0, "MLX90640 RAW FRAME\r\n");

    for (int i = 0; i < MLX90640_PIXEL_COUNT; i++)
    {
        uartPrintf(0, "[%03d] %u\r\n", i, frame[i]);
    }
}
#endif 





