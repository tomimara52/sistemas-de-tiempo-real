#include "desafio_queue.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "stm32h5xx_nucleo.h"

QueueHandle_t ledQueue = NULL;

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} LedStruct;

LedStruct red_led = {.port = GPIOB, .pin = GPIO_PIN_13};

LedStruct green_led = {.port = GPIOB, .pin = GPIO_PIN_14};

LedStruct blue_led = {.port = GPIOB, .pin = GPIO_PIN_15};

LedStruct yellow_led = {.port = GPIOB, .pin = GPIO_PIN_2};

static void blinky_queue() {
    LedStruct *led_p;

    for (;;) {
        led_p = NULL;
        xQueueReceive(ledQueue, &led_p, portMAX_DELAY);

        HAL_GPIO_TogglePin(led_p->port, led_p->pin);
    }
}

static void blinky_admin() {
    for (;;) {
        LedStruct* led = &yellow_led;
        xQueueSend(ledQueue, (void*)&led, 0);

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void desafio_queue() {
    ledQueue = xQueueCreate(3, sizeof(LedStruct *));
    xTaskCreate(blinky_queue, "blinky", 500, NULL, 0, NULL);
    xTaskCreate(blinky_admin, "admin", 500, NULL, 1, NULL);
}
