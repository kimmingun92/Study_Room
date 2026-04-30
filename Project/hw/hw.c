#include "hw.h"

void hwInit(void)
{
    uartInit();
    cliInit();
    logInit();
    dhtInit();
    ledInit();
    servoInit();
    irSensorInit();
}
