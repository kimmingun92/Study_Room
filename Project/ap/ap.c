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

void thermalSystemTask(void *argument)
{
    thermalMain();
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

void rfidSystemTask(void *argument)
{
    for (;;) {
        rfidProcess();
        osDelay(10);
    }
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