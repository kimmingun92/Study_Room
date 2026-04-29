#include "ap.h"

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
}
