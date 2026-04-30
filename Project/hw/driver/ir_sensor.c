#include "ir_sensor.h"

static bool irMonitorState = IR_MON_OFF;
static uint32_t irPeriod = 1000;

void irSensorInit(void)
{
    irMonitorState = IR_MON_OFF;
}

GPIO_PinState irSensorReadRaw(void)
{
    return HAL_GPIO_ReadPin(IR_SENSOR_GPIO_Port, IR_SENSOR_Pin);
}

bool irSensorIsDetected(void)
{
    return (irSensorReadRaw() == GPIO_PIN_RESET);
}

void irSensorSetMonitor(bool state)
{
    irMonitorState = state;
}

bool irSensorGetMonitor(void)
{
    return irMonitorState;
}

void irSensorSetPeriod(uint32_t period)
{
    irPeriod = period;
}

uint32_t irSensorGetPeriod(void)
{
    return irPeriod;
}

void irSensorPrintValue(void)
{
    GPIO_PinState raw = irSensorReadRaw();

    cliPrintf("IR RAW: %d\r\n", raw);

    if (irSensorIsDetected())
        cliPrintf("IR DETECTED\r\n");
    else
        cliPrintf("IR CLEAR\r\n");
}

void irSensorMain(void)
{
    static uint32_t pre_time = 0;

    if (irMonitorState == IR_MON_OFF)
        return;

    if (HAL_GetTick() - pre_time >= irPeriod)
    {
        pre_time = HAL_GetTick();

        irSensorPrintValue();

        cliPrintf("CLI> ");
    }
}