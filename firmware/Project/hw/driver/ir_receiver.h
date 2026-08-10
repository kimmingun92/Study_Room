#ifndef __HW_DRIVER_IR_RECEIVER_H_
#define __HW_DRIVER_IR_RECEIVER_H_

#include "hw_def.h"
#include "def.h"

#define IR_RX_PIN GPIO_PIN_0

// start signal: 13.5ms
#define NEC_LEAD_MIN 12000
#define NEC_LEAD_MAX 15000

// bit0: 1.12ms
#define NEC_BIT0_MIN 800
#define NEC_BIT0_MAX 1400

// bit1: 2.25ms 
#define NEC_BIT1_MIN 1800
#define NEC_BIT1_MAX 2600

#define NEC_REPEAT_MIN 10500
#define NEC_REPEAT_MAX 12000

void irReceiverInit(void);
bool irReceiverAvailable(void);
uint32_t irReceiverGetCode(void);
void irReceiverExtiCallback(uint16_t GPIO_Pin);

#endif