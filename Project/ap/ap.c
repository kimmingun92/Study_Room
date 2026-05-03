#include "ap.h"

extern struct netif gnetif;




void StartDefaultTask(void *argument)
{
    MX_LWIP_Init();
    apInit();
    cliPrintf("CLI> ");
    for (;;) {
        apMain();
    }
}

void dhtSystemTask(void *argument)
{
    for (;;) {
        dhtMain();
        osDelay(10);
    }
}

void irSensorSystemTask(void *argument)
{
    for (;;) {
        irSensorMain();
        osDelay(10);
    }
}

void tcpClientSystemTask(void *argument)
{
    tcpMain();
}

void apInit()
{
    hwInit();
}

void apMain()
{
    cliMain();
    osDelay(10);
}
