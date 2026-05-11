#include "FreeRTOS.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"
#include "queue.h"
#include "tim.h"

QueueHandle_t rpm_queue = NULL;

void rpm_calc(void* args) {
    uint32_t prev_capture = 0;
    uint32_t curr_capture;

    for (;;) {
        xQueueReceive(rpm_queue, &curr_capture, portMAX_DELAY);

        uint32_t period = curr_capture - prev_capture;
        float rpm = (1000000.0f / period) * 60.0f;
        prev_capture = curr_capture;

        printf("rpm: %d.%02d\n", (int)rpm, ((int)(rpm*100)) % 100);
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        uint32_t capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        BaseType_t xHigherPriorityTaskWoken;

        xQueueSendFromISR(rpm_queue, &capture, &xHigherPriorityTaskWoken);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

int my_main() {
    rpm_queue = xQueueCreate(10, sizeof(uint32_t));

    xTaskCreate(rpm_calc, "rpm", 300, NULL, 0, NULL);

    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

    vTaskStartScheduler();

    return 0;
}
