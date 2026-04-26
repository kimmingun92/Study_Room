#include "ap.h"
#include "survo.h"

void apInit(){

}

void apMain(){
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  // Green (LD1)
    HAL_Delay(300);
    
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);  // Blue (LD2)
    HAL_Delay(300);
    
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); // Red (LD3)
    HAL_Delay(300);
}