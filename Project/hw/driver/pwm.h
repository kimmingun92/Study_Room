#ifndef PWM_H
#define PWM_H

#include "hw_def.h"

void pwmStart(void);
void pwmWriteUs(TIM_HandleTypeDef *htim, uint32_t channel, uint16_t pulse_us);

#endif