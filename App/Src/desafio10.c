#include "desafio10.h"
#include "FreeRTOS.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"

static void blinky() {
    TickType_t start_time = xTaskGetTickCount();
    TickType_t ticks_delay = pdMS_TO_TICKS(150);

    while (pdTICKS_TO_MS(xTaskGetTickCount() - start_time) < 10000) {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
        vTaskDelay(ticks_delay);
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);

        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
        vTaskDelay(ticks_delay);
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);

        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
        vTaskDelay(ticks_delay);
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);

        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
        vTaskDelay(ticks_delay);
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);

        vTaskDelay(ticks_delay);
    }

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, 1);
    vTaskDelete(NULL);
}

void blinky_monitor(void *blinky_handle_void) {
    TaskHandle_t blinky_handle = (TaskHandle_t)blinky_handle_void;

    for (;;) {
        TaskStatus_t status;

        vTaskGetInfo(blinky_handle, &status, pdFALSE, eInvalid);

        switch (status.eCurrentState) {
        case eRunning:
            printf("blinky is running\n");
            break;
        case eReady:
            printf("blinky is ready\n");
            break;
        case eBlocked:
            printf("blinky is blocked\n");
            break;
        case eSuspended:
            printf("blinky is suspended\n");
            break;
        case eDeleted:
            printf("blinky is deleted\n");
            break;
        case eInvalid:
            printf("blinky is invalid\n");
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void desafio10() {
    static TaskHandle_t blinky_handle;

    xTaskCreate(blinky, "blinky", 100, NULL, 0, &blinky_handle);

    xTaskCreate(blinky_monitor, "blinky_monitor", 300, (void*)blinky_handle, 1, NULL);
}
