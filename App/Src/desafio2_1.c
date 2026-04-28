#include "desafio2_1.h"
#include "FreeRTOS.h"
#include "button.h"
#include "semphr.h"
#include "stm32h5xx_nucleo.h"

static SemaphoreHandle_t counter_sem = NULL;
static QueueHandle_t counter_queue = NULL;

static void counter() {
    uint16_t press_count = 0;

    for (;;) {
        xSemaphoreTake(counter_sem, portMAX_DELAY);
        press_count++;

        xQueueSend(counter_queue, &press_count, 0);
    }
}

static void display() {
    uint16_t number;
    for (;;) {
        xQueueReceive(counter_queue, &number, portMAX_DELAY);

        uint16_t red_bit    = number & 0b1000;
        uint16_t green_bit  = number & 0b0100;
        uint16_t blue_bit   = number & 0b0010;
        uint16_t yellow_bit = number & 0b0001;

        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, red_bit);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, green_bit);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, blue_bit);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2,  yellow_bit);
    }
}

void USER_BUTTON_Callback() {
    BaseType_t higher_prio_task_awoken = pdFALSE;

    xSemaphoreGiveFromISR(counter_sem, &higher_prio_task_awoken);

    portYIELD_FROM_ISR(higher_prio_task_awoken);
}

void desafio2_1() {
    counter_sem = xSemaphoreCreateBinary();
    counter_queue = xQueueCreate(10, sizeof(uint16_t));

    UserButton_Init(GPIO_MODE_IT_RISING);

    xTaskCreate(counter, "counter", 300, NULL, 1, NULL);
    xTaskCreate(display, "display", 300, NULL, 0, NULL);
}
