#include "motor_r300.h"

extern TIM_HandleTypeDef htim9;

#define MOTOR_R300_TIM      (&htim9)
#define MOTOR_R300_CHANNEL  TIM_CHANNEL_1
#define MOTOR_R300_IN1_PORT  IN1_GPIO_Port
#define MOTOR_R300_IN1_PIN  IN1_Pin
#define MOTOR_R300_IN2_PORT IN2_GPIO_Port
#define MOTOR_R300_IN2_PIN  IN2_Pin

static uint8_t motor_temp_min = 20;
static uint8_t motor_temp_max = 35;
static uint8_t motor_hum_min = 40;
static uint8_t motor_hum_max = 100;
static uint16_t motor_off_us = 0;
static uint16_t motor_on_us = 19999;
static bool motor_is_on = false;

static bool motor_force_off = false;

bool motorR300Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = MOTOR_R300_IN1_PIN;
    HAL_GPIO_Init(MOTOR_R300_IN1_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MOTOR_R300_IN2_PIN;
    HAL_GPIO_Init(MOTOR_R300_IN2_PORT, &GPIO_InitStruct);

    HAL_TIM_PWM_Start(MOTOR_R300_TIM, MOTOR_R300_CHANNEL);
    motorR300Set(false);

    return true;
}


void motorR300Set(bool on)
{
    motor_is_on = on;

    if (on)
    {
       HAL_GPIO_WritePin(MOTOR_R300_IN1_PORT, MOTOR_R300_IN1_PIN, GPIO_PIN_SET);
       HAL_GPIO_WritePin(MOTOR_R300_IN2_PORT, MOTOR_R300_IN2_PIN, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(MOTOR_R300_IN1_PORT, MOTOR_R300_IN1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_R300_IN2_PORT, MOTOR_R300_IN2_PIN, GPIO_PIN_RESET);
    }

    pwmWriteUs(MOTOR_R300_TIM, MOTOR_R300_CHANNEL, on ? motor_on_us : motor_off_us);
}


void motorR300Update(uint8_t temperature, uint8_t humidity)
{
    if (motor_force_off == true)
    {
        motorR300Set(false);
        return;
    }

    if (temperature < 20 || humidity < 30)
    {
        motorR300Set(false);
    }
    else if (temperature < 23 || humidity < 40)
    {
        motorR300SetPulse(0, 3500);    // 아주 약하게
        motorR300Set(true);
    }
    else if (temperature < 26 || humidity < 55)
    {
        motorR300SetPulse(0, 5000);   // 약하게
        motorR300Set(true);
    }
    else
    {
        motorR300SetPulse(0, 7500);   // 세게
        motorR300Set(true);
    }
}


bool motorR300IsOn(void)
{
    return motor_is_on;
}

uint8_t motorR300GetTempMin(void)
{
    return motor_temp_min;
}

uint8_t motorR300GetTempMax(void)
{
    return motor_temp_max;
}

uint8_t motorR300GetHumMin(void)
{
    return motor_hum_min;
}

uint8_t motorR300GetHumMax(void)
{
    return motor_hum_max;
}

uint16_t motorR300GetOffPulse(void)
{
    return motor_off_us;
}

uint16_t motorR300GetOnPulse(void)
{
    return motor_on_us;
}

void cliMotorR300(uint8_t argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1], "on") == 0)
    {
        motor_force_off = false;
        motorR300Set(true);
        cliPrintf("motor on\r\n");
    }
    else if (argc == 2 && strcmp(argv[1], "off") == 0)
    {
        motor_force_off = true;
        motorR300Set(false);
        cliPrintf("motor off\r\n");
    }
    else if (argc == 2 && strcmp(argv[1], "status") == 0)
    {
        cliPrintf("motor %s pulse off:%d on:%d\r\n",
                  motorR300IsOn() ? "on" : "off",
                  motorR300GetOffPulse(), motorR300GetOnPulse());
    }
    else if (argc == 4 && strcmp(argv[1], "pulse") == 0)
    {
        uint16_t off_us = (uint16_t)atoi(argv[2]);
        uint16_t on_us = (uint16_t)atoi(argv[3]);

        motorR300SetPulse(off_us, on_us);
        cliPrintf("motor pulse off:%d on:%d\r\n", off_us, on_us);
    }
    else
    {
        cliPrintf("Usage: motor on/off/status\r\n");
        cliPrintf("       motor pulse off_us on_us\r\n");
    }
}
