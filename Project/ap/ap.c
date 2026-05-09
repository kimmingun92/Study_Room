#include "ap.h"

extern struct netif gnetif;
static volatile bool init_done = false;

void StartDefaultTask(void *argument)
{
    MX_LWIP_Init();
    apInit();
    init_done = true;
    cliPrintf("CLI> ");
    for (;;) {
        apMain();
    }
}

void thermalSystemTask(void *argument)
{
    while(!init_done){
        osDelay(10);
    }
    osDelay(100);

    thermalMain();
}

void dhtSystemTask(void *argument)
{
    while(!init_done){
        osDelay(10);
    }
    osDelay(100);

    for (;;) {
        dhtMain();
        osDelay(10);
    }
}

void irSensorSystemTask(void *argument)
{
    while(!init_done){
        osDelay(10);
    }
    osDelay(100);

    for (;;) {
        irSensorMain();
        osDelay(10);
    }
}

void tcpClientSystemTask(void *argument)
{
    while(!init_done){
        osDelay(10);
    }
    osDelay(100);

    tcpMain();
}

void rfidSystemTask(void *argument)
{
    while(!init_done){
        osDelay(10);
    }
    osDelay(100);

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