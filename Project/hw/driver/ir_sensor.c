#include "ir_sensor.h"

// D7 = PF13
#define IR_SENSOR_GPIO_PORT GPIOG
#define IR_SENSOR_PIN       GPIO_PIN_0

void irSensorInit(void)
{
}

GPIO_PinState irSensorReadRaw(void)
{
    return HAL_GPIO_ReadPin(IR_SENSOR_GPIO_PORT, IR_SENSOR_PIN);
}

bool irSensorIsDetected(void)
{
    /*
     * 일단 RAW 값만 보고 판단할 거라서
     * HIGH 감지 타입 기준으로 작성
     */
    return (irSensorReadRaw() == GPIO_PIN_RESET);
}

void irSensorMain(void)
{
    static uint32_t pre_time = 0;

    if (HAL_GetTick() - pre_time >= 500)
    {
        pre_time = HAL_GetTick();

        GPIO_PinState raw = irSensorReadRaw();

        cliPrintf("IR RAW: %d\r\n", raw);

        if (irSensorIsDetected())
            cliPrintf("IR DETECTED\r\n");
        else
            cliPrintf("IR CLEAR\r\n");

        cliPrintf("CLI> ");
    }
}