#include "pwm.h"

#define PWM_GPIO_CH_MAX 9
#define PWM_GPIO_STEP_MAX 20
#define PWM_GPIO_TICK_MS 1

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t duty;
    bool is_used;
} pwm_gpio_t;

extern TIM_HandleTypeDef htim3;

static pwm_gpio_t pwm_gpio_tbl[PWM_GPIO_CH_MAX];
static osThreadId_t pwm_gpio_task_handle = NULL;
static const osThreadAttr_t pwm_gpio_task_attributes = {
    .name = "pwmGpio",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};

static void pwmGpioTask(void *argument);

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

bool pwmGpioAttach(uint8_t ch, GPIO_TypeDef *port, uint16_t pin)
{
    if (ch >= PWM_GPIO_CH_MAX || port == NULL) {
        return false;
    }

    pwm_gpio_tbl[ch].port = port;
    pwm_gpio_tbl[ch].pin = pin;
    pwm_gpio_tbl[ch].duty = 0;
    pwm_gpio_tbl[ch].is_used = true;
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);

    if (pwm_gpio_task_handle == NULL) {
        pwm_gpio_task_handle = osThreadNew(pwmGpioTask, NULL, &pwm_gpio_task_attributes);
    }

    return true;
}

void pwmGpioWrite(uint8_t ch, uint8_t duty)
{
    if (ch >= PWM_GPIO_CH_MAX || !pwm_gpio_tbl[ch].is_used) {
        return;
    }

    if (duty > 100) {
        duty = 100;
    }

    pwm_gpio_tbl[ch].duty = duty;
}

static void pwmGpioTask(void *argument)
{
    uint8_t step = 0;
    (void)argument;

    while (1) {
        for (uint8_t i = 0; i < PWM_GPIO_CH_MAX; i++) {
            if (pwm_gpio_tbl[i].is_used) {
                uint8_t threshold = (uint8_t)((pwm_gpio_tbl[i].duty * PWM_GPIO_STEP_MAX) / 100);
                HAL_GPIO_WritePin(pwm_gpio_tbl[i].port, pwm_gpio_tbl[i].pin,
                                  step < threshold ? GPIO_PIN_SET : GPIO_PIN_RESET);
            }
        }

        step++;
        if (step >= PWM_GPIO_STEP_MAX) {
            step = 0;
        }

        osDelay(PWM_GPIO_TICK_MS);
    }
}
