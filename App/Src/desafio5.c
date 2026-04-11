#include "FreeRTOS.h"
#include "button.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"
#include "semphr.h"

typedef struct _blinkyArgs {
    uint32_t ms_delay;
    GPIO_TypeDef* port;
    uint16_t pin;
} BlinkyArg;

void blinky_hal_delay(void *void_args) {
    BlinkyArg args = *((BlinkyArg*) void_args);

    for (;;) {
        HAL_GPIO_TogglePin(args.port, args.pin);

        HAL_Delay(args.ms_delay);
    }
}

SemaphoreHandle_t sem;

void blinky_priorizer(void *void_blinky_handle) {
    TaskHandle_t blinky_handle = (TaskHandle_t) void_blinky_handle;

    for (;;) {
        xSemaphoreTake(sem, portMAX_DELAY);

        printf("increasing blink red priority\n");

        UBaseType_t blinky_priority = uxTaskPriorityGet(blinky_handle);
        vTaskPrioritySet(blinky_handle, blinky_priority + 1);

        vTaskDelay(pdMS_TO_TICKS(3000));

        vTaskPrioritySet(blinky_handle, blinky_priority);
    }
}

void USER_BUTTON_Callback() {
    BaseType_t higher_prio_task_awoken = pdFALSE;

    xSemaphoreGiveFromISR(sem, &higher_prio_task_awoken);

    portYIELD_FROM_ISR(higher_prio_task_awoken);
}

void desafio5(void) {
    static TaskHandle_t red_blinky_handle;
    static BlinkyArg red_led;
    red_led.ms_delay = 300;
    red_led.port = GPIOB;
    red_led.pin = GPIO_PIN_13;
    xTaskCreate(blinky_hal_delay, "red", 100, (void*)&red_led, 0, &red_blinky_handle);

    static BlinkyArg green_led;
    green_led.ms_delay = 300;
    green_led.port = GPIOB;
    green_led.pin = GPIO_PIN_14;
    xTaskCreate(blinky_hal_delay, "green", 100, (void*)&green_led, 0, NULL);

    sem = xSemaphoreCreateBinary();

    UserButton_Init(GPIO_MODE_IT_RISING);

    xTaskCreate(blinky_priorizer, "blinky prorizer", 300, (void*)red_blinky_handle, 2, NULL);
}
