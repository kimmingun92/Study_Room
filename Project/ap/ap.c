#include "ap.h"
#include "stm32f4xx_hal_uart.h"
#include "usart.h"
#include <stdio.h>

extern struct netif gnetif;

void StartDefaultTask(void *argument)
{
    MX_LWIP_Init();
    apInit();
    cliPrintf("CLI> ");
    while (1) {
        apMain();
        osDelay(1);
    }
}

void thermalSystemTask(void *argument)
{
    for(;;)
    {
        thermalMain();
        osDelay(1);
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