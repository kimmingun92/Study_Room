#ifndef __HW_DRIVER_DHT_H_
#define __HW_DRIVER_DHT_H_

#include "hw_def.h"
#include "def.h"

bool dhtInit();
bool dhtRead();
void cliDht(uint8_t argc, char *argv[]);
void dhtMain(void);


uint8_t getTem();
uint8_t getHum();



#endif
