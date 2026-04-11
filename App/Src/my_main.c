#include "FreeRTOS.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"

#include "desafio8.h"

int my_main() {
    
    desafio8();

    vTaskStartScheduler();

    return 0;
}
