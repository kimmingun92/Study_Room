#include "ap.h"
#include "servo.h"

extern struct netif gnetif;

void StartDefaultTask(void *argument)
{
    MX_LWIP_Init();
    apInit();
    cliPrintf("CLI> ");
    while (1) {
        apMain();
    }
}

void apInit()
{
    hwInit();
}

void apMain()
{
    cliMain();
    dhtMain();
    osDelay(10);
}

void StartTask02(void *argument)
{
    while (1) {
        osDelay(10);
    }
}
