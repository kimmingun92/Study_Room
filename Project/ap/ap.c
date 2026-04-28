#include "ap.h"

extern struct netif gnetif;

void StartDefaultTask(void *argument){
    MX_LWIP_Init();
    apInit();
    apMain();
}

void apInit(){
    uartInit();
}

void apMain(){
    while (1) {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
        osDelay(1000);
    }
}
