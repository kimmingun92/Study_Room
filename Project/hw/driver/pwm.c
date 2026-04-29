#include "pwm.h"

extern TIM_HandleTypeDef htim3;

void pwmStart(void)
{
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}

void pwmWriteUs(TIM_HandleTypeDef *htim, uint32_t channel, uint16_t pulse_us)
{
    __HAL_TIM_SET_COMPARE(htim, channel, pulse_us);
}