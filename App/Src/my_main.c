#include "FreeRTOS.h"
#include "button.h"
#include "semphr.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"
#include <stdint.h>

typedef struct _blinky_admin_args {
    SemaphoreHandle_t *sem;
    TickType_t *tick_delay;
    TaskHandle_t *blinky_handle;
} BlinkyAdminArgs;

SemaphoreHandle_t blinky_admin_sem;

void blinky(void *ticks_delay_void) {

    vTaskSuspend(NULL);

    for (;;) {
        BSP_LED_Toggle(LED_GREEN);

        vTaskDelay(*((TickType_t *)ticks_delay_void));
    }
}

void blinky_admin(void *args_void) {
    BlinkyAdminArgs args = *((BlinkyAdminArgs *)args_void);
    uint8_t blinky_is_running = 1;
    TickType_t button_press_tick = 0;

    for (;;) {
        xSemaphoreTake(*args.sem, portMAX_DELAY);

        if (blinky_is_running || button_press_tick == 0) {
            vTaskSuspend(*args.blinky_handle);
            BSP_LED_On(LED_GREEN);

            button_press_tick = xTaskGetTickCount();
            printf("Setting blinky period (t=%lu)\n", button_press_tick);
        } else {

            TickType_t current_tick = xTaskGetTickCount();
            *args.tick_delay = current_tick - button_press_tick;

            vTaskResume(*args.blinky_handle);

            printf("Blinky period set at %lu (t=%lu)\n", *args.tick_delay,
                   current_tick);
        }

        blinky_is_running = !blinky_is_running;
    }
}

void USER_BUTTON_Callback() { 
    BaseType_t higher_prio_task_awoken = pdFALSE;

    xSemaphoreGiveFromISR(blinky_admin_sem, &higher_prio_task_awoken);

    portYIELD_FROM_ISR(higher_prio_task_awoken);
}

int my_main() {
    UserButton_Init(GPIO_MODE_IT_RISING_FALLING);

    blinky_admin_sem = xSemaphoreCreateBinary();

    static TickType_t blinky_ticks_delay = 0;

    static TaskHandle_t blinky_handle;

    static BlinkyAdminArgs blinky_admin_args;
    blinky_admin_args.tick_delay = &blinky_ticks_delay;
    blinky_admin_args.sem = &blinky_admin_sem;
    blinky_admin_args.blinky_handle = &blinky_handle;

    xTaskCreate(blinky, "blinky", 100, &blinky_ticks_delay, 0, &blinky_handle);
    xTaskCreate(blinky_admin, "blinky admin", 1000, &blinky_admin_args, 1, NULL);

    vTaskStartScheduler();

    return 0;
}
