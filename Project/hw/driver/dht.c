#include "dht.h"

#define DHT_PORT GPIOA
#define DHT_PIN  GPIO_PIN_3

static uint8_t dht_tem = 0;
static uint8_t dht_hum = 0;

extern TIM_HandleTypeDef htim2;

static void delay_us(uint32_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (__HAL_TIM_GET_COUNTER(&htim2) < us)
    {
    }
}

static bool waitPinChange(GPIO_PinState state, uint32_t timeout_us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) == state)
    {
        if (__HAL_TIM_GET_COUNTER(&htim2) > timeout_us)
        {
            return false;
        }
    }

    return true;
}

static void dhtSetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
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

bool dhtInit(void)
{
    HAL_TIM_Base_Start(&htim2);
    dhtSetInput();

    return true;
}

static bool dhtReadBit(uint8_t *bit)
{
    uint32_t high_time = 0;

    if (!waitPinChange(GPIO_PIN_RESET, 100))
    {
        return false;
    }

    __HAL_TIM_SET_COUNTER(&htim2, 0);

    while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) == GPIO_PIN_SET)
    {
        high_time = __HAL_TIM_GET_COUNTER(&htim2);

        if (high_time > 120)
        {
            return false;
        }
    }

    if (high_time > 40)
    {
        *bit = 1;
    }
    else
    {
        *bit = 0;
    }

    return true;
}

bool dhtRead(void)
{
    uint8_t data[5] = {0};
    uint8_t bit = 0;

    dhtSetOutput();

    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
    HAL_Delay(2);

    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
    delay_us(30);

    dhtSetInput();

    if (!waitPinChange(GPIO_PIN_SET, 100))
    {
        return false;
    }

    if (!waitPinChange(GPIO_PIN_RESET, 100))
    {
        return false;
    }

    if (!waitPinChange(GPIO_PIN_SET, 100))
    {
        return false;
    }

    for (int i = 0; i < 40; i++)
    {
        if (!dhtReadBit(&bit))
        {
            return false;
        }

        data[i / 8] <<= 1;
        data[i / 8] |= bit;
    }

    uint8_t checksum = data[0] + data[1] + data[2] + data[3];

    if (checksum != data[4])
    {
        return false;
    }

    dht_hum = data[0];
    dht_tem = data[2];

    return true;
}

uint8_t getTem(void)
{
    return dht_tem;
}

uint8_t getHum(void)
{
    return dht_hum;
}