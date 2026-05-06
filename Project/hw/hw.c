#include "hw.h"

void hwInit(void)
{
    uartInit();
    cliInit();
    logInit();
    thermalInit();
    oledInit();
}