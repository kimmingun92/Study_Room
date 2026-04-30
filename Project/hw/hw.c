#include "hw.h"

void hwInit(void)
{
    uartInit();
    dhtInit();
    cliInit();
    ledInit();
    logInit();
    servoInit();
    motorR300Init();
}
