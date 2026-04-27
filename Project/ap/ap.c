#include "ap.h"
#include "led.h"

void apInit(){
    ledInit();
}

void apMain(){
    static uint8_t led_init_done = 0;

    if (led_init_done == 0) {
        ledInit();
        led_init_done = 1;
    }

    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  // Green (LD1)
    HAL_Delay(300);
    
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);  // Blue (LD2)
    HAL_Delay(300);
    
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); // Red (LD3)
    HAL_Delay(300);

}
