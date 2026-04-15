#include "FreeRTOS.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"

#include "desafio_queue.h"

int my_main() {

    desafio_queue();

    vTaskStartScheduler();

    return 0;
}
