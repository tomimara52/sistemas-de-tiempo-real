#include "FreeRTOS.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"

#include "desafio10.h"

int my_main() {
    
    desafio10();

    vTaskStartScheduler();

    return 0;
}
