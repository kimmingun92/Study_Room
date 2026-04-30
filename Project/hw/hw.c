#include "hw.h"

void hwInit(void)
{
    uartInit();
    dhtInit();
    cliInit();
    logInit();
    servoInit();
    irSensorInit();
}
