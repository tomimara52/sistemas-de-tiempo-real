#include "FreeRTOS.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"

#include "desafio5.h"

typedef struct _blinkyArgs {
    uint32_t ms_delay;
    GPIO_TypeDef* port;
    uint16_t pin;
} BlinkyArg;

void blinky(void *void_args) {
    BlinkyArg args = *((BlinkyArg*) void_args);

    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        HAL_GPIO_TogglePin(args.port, args.pin);

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(args.ms_delay));
    }
}



int my_main() {
    
    desafio5();

    vTaskStartScheduler();

    return 0;
}
