#include "hw.h"

void hwInit(void)
{
    uartInit();
    cliInit();
    ledInit();
    logInit();
    dhtInit();
    ledInit();
    servoInit();
    irSensorInit();
    motorR300Init();
    rfidInit(); 
    thermalInit();
    oledInit();
}
