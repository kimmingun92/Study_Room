#ifndef PWM_H
#define PWM_H

#include "hw_def.h"

void pwmStart(void);
void pwmWriteUs(TIM_HandleTypeDef *htim, uint32_t channel, uint16_t pulse_us);
bool pwmGpioAttach(uint8_t ch, GPIO_TypeDef *port, uint16_t pin);
void pwmGpioWrite(uint8_t ch, uint8_t duty);

#endif
