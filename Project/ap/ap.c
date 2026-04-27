#include "ap.h"

void apInit(){


     dhtInit();

     
}


/*
void apMain(){


{
    uint32_t prev_time = 0;

    while (1)
    {
        if (HAL_GetTick() - prev_time >= 2000)
        {
            prev_time = HAL_GetTick();

            if (dhtRead())
            {
                printf("Temp: %d C, Hum: %d %%\r\n", getTem(), getHum());
            }
            else
            {
                printf("DHT22 read failed\r\n");
            }
        }
    }
}
}

*/


void apMain()
{
    static uint32_t prev_time = 0;

    if (HAL_GetTick() - prev_time >= 2000)
    {
        prev_time = HAL_GetTick();

        printf("DHT read start\r\n");

        if (dhtRead())
        {
            printf("Temp: %d C, Hum: %d %%\r\n", getTem(), getHum());
        }
        else
        {
            printf("DHT22 read failed\r\n");
        }
    }
}



int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 10);
    return ch;
}
