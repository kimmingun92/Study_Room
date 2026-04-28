#include "ap.h"
#include "stm32f4xx_hal.h"
#include "uart.h"

void apInit(){

    uartInit();
    dhtInit();

    uartPrintf(0, "AP Init OK\r\n");
     
}



void apMain(void)
{
    while (1)
    {
        if (dhtRead())
        {
            uartPrintf(0, "TEMP : %d C\r\n", getTem());
            uartPrintf(0, "HUM  : %d %%\r\n", getHum());
        }
        else
        {
            uartPrintf(0, "DHT22 read failed\r\n");
        }

        osDelay(2000);
    }
}