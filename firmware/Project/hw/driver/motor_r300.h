#ifndef __HW_DRIVER_MOTOR_R300_H_
#define __HW_DRIVER_MOTOR_R300_H_

#include "hw_def.h"

bool motorR300Init(void);
void motorR300Set(bool on);
void motorR300Update(uint8_t temperature, uint8_t humidity);
bool motorR300IsOn(void);
void motorR300SetPulse(uint16_t off_us, uint16_t on_us);
uint8_t motorR300GetTempMin(void);
uint8_t motorR300GetTempMax(void);
uint8_t motorR300GetHumMin(void);
uint8_t motorR300GetHumMax(void);
uint16_t motorR300GetOffPulse(void);
uint16_t motorR300GetOnPulse(void);
void cliMotorR300(uint8_t argc, char *argv[]);

#endif
