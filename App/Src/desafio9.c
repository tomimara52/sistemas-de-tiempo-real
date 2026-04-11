#include "desafio9.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"
#include "button.h"

void blinky() {
    TickType_t blink_time = pdMS_TO_TICKS(200);

    for (;;) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
        vTaskDelay(blink_time);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);

        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, 1);
        vTaskDelay(blink_time);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, 0);

        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, 1);
        vTaskDelay(blink_time);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, 0);

        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
        vTaskDelay(blink_time);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);

        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, 1);
        vTaskDelay(blink_time);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, 0);

        vTaskDelay(blink_time);
    }
}

static SemaphoreHandle_t sem;

void blinky_admin(void* blinky_handle_void) {
    uint8_t blinky_running = 1;
    TaskHandle_t blinky_handle = (TaskHandle_t) blinky_handle_void;

    for (;;) {
        xSemaphoreTake(sem, portMAX_DELAY);

        if (blinky_running) {
            vTaskSuspend(blinky_handle);
        } else {
            vTaskResume(blinky_handle);
        }

        blinky_running = !blinky_running;
    }
}

void USER_BUTTON_Callback() {
    BaseType_t higher_prio_task_awoken = pdFALSE;

    xSemaphoreGiveFromISR(sem, &higher_prio_task_awoken);

    portYIELD_FROM_ISR(higher_prio_task_awoken);
}

void desafio9() {
    UserButton_Init(GPIO_MODE_IT_RISING);

    sem = xSemaphoreCreateBinary();

    static TaskHandle_t blinky_handle;
    xTaskCreate(blinky, "blinky", 100, NULL, 0, &blinky_handle);
    xTaskCreate(blinky_admin, "blinky admin", 500, (void*)blinky_handle, 1, NULL);
}
