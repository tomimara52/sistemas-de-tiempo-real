#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"

static SemaphoreHandle_t sem;

void blinky_seq(void *void_args) {
    for (;;) {
        xSemaphoreTake(sem, portMAX_DELAY);

        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
        vTaskDelay(pdMS_TO_TICKS(150));
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);

        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
        vTaskDelay(pdMS_TO_TICKS(150));
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);

        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
        vTaskDelay(pdMS_TO_TICKS(150));
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);

        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
        vTaskDelay(pdMS_TO_TICKS(150));
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);

        xSemaphoreGive(sem);

        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void blinky_par(void *void_args) {
    for (;;) {
        xSemaphoreTake(sem, portMAX_DELAY);

        for (uint32_t i = 0; i < 8; ++i) {
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);

            if (i != 7)
                vTaskDelay(pdMS_TO_TICKS(150));
        }

        xSemaphoreGive(sem);

        vTaskDelay(pdMS_TO_TICKS(150));
    }
}
void desafio8() {
    sem = xSemaphoreCreateMutex();

    xTaskCreate(blinky_seq, "blinky_seq", 100, NULL, 0, NULL);
    xTaskCreate(blinky_par, "blinky_par", 100, NULL, 0, NULL);
}
