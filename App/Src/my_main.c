#include "FreeRTOS.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"

#include "desafio2_2.h"

int my_main() {

    desafio2_2();

    vTaskStartScheduler();

    return 0;
}
