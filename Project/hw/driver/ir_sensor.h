#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include "hw_def.h"

#define IR_MON_OFF false
#define IR_MON_ON  true

void irSensorInit(void);
void irSensorMain(void);

GPIO_PinState irSensorReadRaw(void);
bool irSensorIsDetected(void);

void irSensorSetMonitor(bool state);
bool irSensorGetMonitor(void);

void irSensorSetPeriod(uint32_t period);
uint32_t irSensorGetPeriod(void);

void irSensorPrintValue(void);

#endif