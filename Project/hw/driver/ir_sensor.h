#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include "hw_def.h"
#include "def.h"

void irSensorInit(void);
void irSensorMain(void);

GPIO_PinState irSensorReadRaw(void);
bool irSensorIsDetected(void);

#endif