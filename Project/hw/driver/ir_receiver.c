#include "ir_receiver.h"
#include "tim.h"
#include "main.h"

static volatile uint32_t ir_data = 0;
static volatile bool ir_ready = false;

static volatile uint32_t temp_code = 0;
static volatile uint8_t bit_count = 0;

static uint32_t necConvertToNormalOrder(uint32_t raw)
{
    uint8_t addr     = (raw >> 0)  & 0xFF;
    uint8_t inv_addr = (raw >> 8)  & 0xFF;
    uint8_t cmd      = (raw >> 16) & 0xFF;
    uint8_t inv_cmd  = (raw >> 24) & 0xFF;

    return ((uint32_t)addr << 24) |
           ((uint32_t)inv_addr << 16) |
           ((uint32_t)cmd << 8) |
           ((uint32_t)inv_cmd);
}

void irReceiverInit(void)
{
    ir_data = 0;
    ir_ready = false;
    temp_code = 0;
    bit_count = 0;

    __HAL_TIM_SET_COUNTER(&htim4, 0);
    HAL_TIM_Base_Start(&htim4);
}

bool irReceiverAvailable(void)
{
    return ir_ready;
}

uint32_t irReceiverGetCode(void)
{
    uint32_t ret;

    __disable_irq();
    ret = ir_data;
    ir_ready = false;
    __enable_irq();

    return ret;
}

void irReceiverExtiCallback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin != IR_RX_PIN)
    {
        return;
    }

    uint32_t duration = __HAL_TIM_GET_COUNTER(&htim4);
    __HAL_TIM_SET_COUNTER(&htim4, 0);

    // NEC Lead Code: 9ms LOW + 4.5ms HIGH = 약 13.5ms
    if (duration > NEC_LEAD_MIN && duration < NEC_LEAD_MAX)
    {
        bit_count = 0;
        temp_code = 0;
        return;
    }

    // NEC Repeat Code: 버튼을 길게 누를 때 발생
    if (duration > NEC_REPEAT_MIN && duration < NEC_REPEAT_MAX)
    {
        // 지금은 repeat 처리 안 함
        return;
    }

    if (bit_count >= 32)
    {
        bit_count = 0;
        temp_code = 0;
        return;
    }

    // NEC bit 0: 약 1.12ms
    if (duration > NEC_BIT0_MIN && duration < NEC_BIT0_MAX)
    {
        // 0은 temp_code에 저장할 필요 없음
        bit_count++;
    }
    // NEC bit 1: 약 2.25ms
    else if (duration > NEC_BIT1_MIN && duration < NEC_BIT1_MAX)
    {
        temp_code |= (1UL << bit_count);
        bit_count++;
    }
    else
    {
        // 이상한 타이밍이면 수신 초기화
        bit_count = 0;
        temp_code = 0;
        return;
    }

    if (bit_count == 32)
    {
        uint8_t addr     = (temp_code >> 0)  & 0xFF;
        uint8_t inv_addr = (temp_code >> 8)  & 0xFF;
        uint8_t cmd      = (temp_code >> 16) & 0xFF;
        uint8_t inv_cmd  = (temp_code >> 24) & 0xFF;

        // NEC 정상 데이터인지 검증
        if (((uint8_t)(addr ^ inv_addr) == 0xFF) &&
            ((uint8_t)(cmd ^ inv_cmd) == 0xFF))
        {
            ir_data = necConvertToNormalOrder(temp_code);
            ir_ready = true;
        }

        bit_count = 0;
        temp_code = 0;
    }
}

/*
 * HAL GPIO EXTI Callback
 * PA0 인터럽트가 발생하면 HAL에서 이 함수로 들어온다.
 * 버튼을 사용하지 않고 IR Receiver만 사용하므로 여기서 바로 IR 콜백으로 넘긴다.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    irReceiverExtiCallback(GPIO_Pin);
}