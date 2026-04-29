#include "hw.h"

void hwInit(void)
{
    uartInit();
    cliInit();
    ledInit();
    logInit();
}
