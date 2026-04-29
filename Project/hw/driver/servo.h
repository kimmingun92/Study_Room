#ifndef __HW_DRIVER_SERVO_H_
#define __HW_DRIVER_SERVO_H_

#include "hw_def.h"
#include "pwm.h"

#define DOOR_CLOSE false
#define DOOR_OPEN  true

#define SERVO_COUNT 4

void servoInit(void);
void changeDoorState(uint8_t id, bool doorState);
bool getDoorState(uint8_t id);

#endif