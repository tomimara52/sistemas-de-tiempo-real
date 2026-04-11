#include "FreeRTOS.h"
#include "button.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"

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

void USER_BUTTON_Callback() {
    printf("hola\n");
}

int my_main() {

    static BlinkyArg red_led;
    red_led.ms_delay = 250;
    red_led.port = GPIOB;
    red_led.pin = GPIO_PIN_13;
    xTaskCreate(blinky, "red", 100, (void*)&red_led, 0, NULL);

    static BlinkyArg green_led;
    green_led.ms_delay = 500;
    green_led.port = GPIOB;
    green_led.pin = GPIO_PIN_14;
    xTaskCreate(blinky, "green", 100, (void*)&green_led, 0, NULL);

    static BlinkyArg blue_led;
    blue_led.ms_delay = 1000;
    blue_led.port = GPIOB;
    blue_led.pin = GPIO_PIN_15;
    xTaskCreate(blinky, "blue", 100, (void*)&blue_led, 0, NULL);

    static BlinkyArg yellow_led;
    yellow_led.ms_delay = 2000;
    yellow_led.port = GPIOB;
    yellow_led.pin = GPIO_PIN_2;
    xTaskCreate(blinky, "yellow", 100, (void*)&yellow_led, 0, NULL);

    vTaskStartScheduler();

    return 0;
}
