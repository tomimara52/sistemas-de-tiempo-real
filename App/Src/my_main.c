#include "FreeRTOS.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"

#include "desafio9.h"

int my_main() {
    
    desafio9();

    vTaskStartScheduler();

    return 0;
}
