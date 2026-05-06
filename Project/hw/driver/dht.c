#include "dht.h"

#include "motor_r300.h"


#define DHT_PORT GPIOA
#define DHT_PIN GPIO_PIN_3

static uint8_t dht_tem = 0;
static uint8_t dht_hum = 0;
static volatile bool dht_print_on = false;
static volatile uint32_t dht_interval_ms = 2000;
static volatile uint32_t dht_prev_time = 0;
bool isAutoMotor = true;

extern TIM_HandleTypeDef htim2;

static void delay_us(uint32_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (__HAL_TIM_GET_COUNTER(&htim2) < us) {
    }
}

static void dhtSetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}

static void dhtSetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}

static bool waitPin(GPIO_PinState state, uint32_t timeout_us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) != state) {
        if (__HAL_TIM_GET_COUNTER(&htim2) > timeout_us) {
            return false;
        }
    }

    return true;
}

bool dhtInit(void)
{
    HAL_TIM_Base_Start(&htim2);
    dhtSetInput();

    return true;
}

bool dhtRead(void)
{
    uint8_t data[5] = {0};

    dhtSetOutput();

    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
    delay_us(1200);

    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
    delay_us(30);

    dhtSetInput();

    if (waitPin(GPIO_PIN_RESET, 100) == false)
        return false;
    if (waitPin(GPIO_PIN_SET, 100) == false)
        return false;
    if (waitPin(GPIO_PIN_RESET, 100) == false)
        return false;

    for (int i = 0; i < 40; i++) {
        if (waitPin(GPIO_PIN_SET, 100) == false)
            return false;

        __HAL_TIM_SET_COUNTER(&htim2, 0);

        if (waitPin(GPIO_PIN_RESET, 120) == false)
            return false;

        uint32_t high_time = __HAL_TIM_GET_COUNTER(&htim2);

        data[i / 8] <<= 1;

        if (high_time > 40) {
            data[i / 8] |= 1;
        }
    }

    uint8_t checksum = data[0] + data[1] + data[2] + data[3];

    if (checksum != data[4]) {
        return false;
    }

    uint16_t raw_hum = (data[0] << 8) | data[1];
    uint16_t raw_tem = (data[2] << 8) | data[3];

    dht_hum = raw_hum / 10;

    if (raw_tem & 0x8000) {
        raw_tem &= 0x7FFF;
        dht_tem = raw_tem / 10;
    } else {
        dht_tem = raw_tem / 10;
    }

    return true;
}

void cliDht(uint8_t argc, char *argv[])
{
    if (argc == 1) {
        dht_interval_ms = 2000;
        dht_print_on = true;
        dht_prev_time = millis() - dht_interval_ms;
        cliPrintf("dht on %dms\r\n", dht_interval_ms);
    } else if (argc == 2 && strcmp(argv[1], "off") == 0) {
        dht_print_on = false;
        cliPrintf("dht off\r\n");
    } else if (argc == 2 && strcmp(argv[1], "status") == 0) {
        cliPrintf("dht %s %dms\r\n", dht_print_on ? "on" : "off", dht_interval_ms);
        cliPrintf("range T:%d~%d H:%d~%d\r\n", motorR300GetTempMin(), motorR300GetTempMax(),
                  motorR300GetHumMin(), motorR300GetHumMax());
        cliPrintf("motor %s pulse off:%d on:%d\r\n", motorR300IsOn() ? "on" : "off",
                  motorR300GetOffPulse(), motorR300GetOnPulse());
    } else if (argc == 2) {
        uint32_t interval_ms = (uint32_t)atoi(argv[1]);

        if (interval_ms < 2000) {
            cliPrintf("Usage: dht [period_ms >= 2000]\r\n");
            cliPrintf("       dht off\r\n");
            return;
        }

        dht_interval_ms = interval_ms;
        dht_print_on = true;
        dht_prev_time = millis() - dht_interval_ms;
        cliPrintf("dht on %dms\r\n", dht_interval_ms);
    } else {
        cliPrintf("Usage: dht [period_ms >= 2000]\r\n");
        cliPrintf("       dht status\r\n");
        cliPrintf("       dht off\r\n");
    }
}

void dhtMain(void)
{
    if (millis() - dht_prev_time >= dht_interval_ms) {
        dht_prev_time = millis();

        if (dhtRead()) {
            if (isAutoMotor) {
                motorR300Update(getTem(), getHum());
            }

            if (dht_print_on == true) {
                cliPrintf("$%d,%d#\r\n", getTem(), getHum());
            }
        } else {
            if (dht_print_on == true) {
                cliPrintf("$Error#\r\n");
            }
        }
    }
}

uint8_t getTem(void)
{
    return dht_tem;
}

uint8_t getHum(void)
{
    return dht_hum;
}
