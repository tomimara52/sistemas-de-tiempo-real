#include "desafio2_1.h"
#include "FreeRTOS.h"
#include "button.h"
#include "semphr.h"
#include "stm32h5xx_nucleo.h"

static QueueHandle_t action_queue = NULL;

enum Command {
    ON = 1,
    OFF = 0,
};

typedef struct {
    enum Command command;
    GPIO_TypeDef* port;
    uint16_t pin;
} LedAction;


static void display() {
    LedAction action;

    for (;;) {
        xQueueReceive(action_queue, &action, portMAX_DELAY);

        HAL_GPIO_WritePin(action.port, action.pin, (GPIO_PinState)action.command);
    }
}


static void controller() {
    LedAction action;
    action.port = GPIOB;

    for (;;) {
        action.command = ON;
        action.pin = GPIO_PIN_13;
        xQueueSend(action_queue, &action, 0);

        vTaskDelay(pdMS_TO_TICKS(300));


        action.pin = GPIO_PIN_14;
        xQueueSend(action_queue, &action, 0);

        vTaskDelay(pdMS_TO_TICKS(300));


        action.command = OFF;
        action.pin = GPIO_PIN_13;
        xQueueSend(action_queue, &action, 0);
        action.command = ON;
        action.pin = GPIO_PIN_15;
        xQueueSend(action_queue, &action, 0);

        vTaskDelay(pdMS_TO_TICKS(300));


        action.command = OFF;
        action.pin = GPIO_PIN_14;
        xQueueSend(action_queue, &action, 0);
        action.command = ON;
        action.pin = GPIO_PIN_2;
        xQueueSend(action_queue, &action, 0);

        vTaskDelay(pdMS_TO_TICKS(300));


        action.command = OFF;
        action.pin = GPIO_PIN_15;
        xQueueSend(action_queue, &action, 0);

        vTaskDelay(pdMS_TO_TICKS(300));


        action.pin = GPIO_PIN_2;
        xQueueSend(action_queue, &action, 0);

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}


void desafio2_2() {
    action_queue = xQueueCreate(10, sizeof(LedAction));

    xTaskCreate(display, "display", 400, NULL, 0, NULL);
    xTaskCreate(controller, "controller", 400, NULL, 1, NULL);
}
