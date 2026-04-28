#ifndef __HW_DRIVER_DHT_H_
#define __HW_DRIVER_DHT_H_

#include "hw_def.h"
#include "def.h"

bool dhtInit();
bool dhtRead();


uint8_t getTem();
uint8_t getHum();



#endif