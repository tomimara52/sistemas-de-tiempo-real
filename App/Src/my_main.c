#include "FreeRTOS.h"
#include "button.h"
#include "queue.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"
#include "tim.h"

// timer clock: 16Mhz
// prescaler: 15
// => frequency: 16Mhz / 16 = 1Mhz
const uint32_t TIMER_FREQ = 1000000;
const float TIMER_FREQ_FLOAT = (float)TIMER_FREQ;
const uint32_t MAX_RPM = 6000; // 100 RPS
const uint32_t MIN_PERIOD = (TIMER_FREQ * 60) / MAX_RPM;
const float TARGET_THRESH = 0.1f;

QueueHandle_t rpm_queue = NULL;

enum Command {
    NEW_CAPTURE = 0,
    SET_TARGET = 1,
};

typedef struct {
    enum Command command;
    uint32_t payload;
} Event;

void rpm_calc(void *args) {
    uint32_t prev_capture = 0;
    float rpm = 0.0f;
    float target_rpm = 0.0f;
    Event event;

    for (;;) {
        xQueueReceive(rpm_queue, &event, portMAX_DELAY);

        if (event.command == NEW_CAPTURE) {
            uint32_t curr_capture = event.payload;
            uint32_t period = curr_capture - prev_capture;

            if (period < MIN_PERIOD)
                continue;

            rpm = (TIMER_FREQ_FLOAT / period) * 60.0f;
            prev_capture = curr_capture;

            printf("rpm: %d.%02d\t", (int)rpm, ((int)(rpm * 100)) % 100);
            printf("period: %d\n", period);

            if (target_rpm != 0 &&
                (target_rpm - rpm) * (target_rpm - rpm) >
                    rpm * rpm * TARGET_THRESH * TARGET_THRESH) {
                HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
            } else {
                HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
            }
        } else if (event.command == SET_TARGET) {
            target_rpm = rpm;
        }
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        Event capture_event = {.command = NEW_CAPTURE};
        capture_event.payload = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        BaseType_t xHigherPriorityTaskWoken;

        xQueueSendFromISR(rpm_queue, &capture_event, &xHigherPriorityTaskWoken);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void USER_BUTTON_Callback() {
    Event set_target_event = {.command = SET_TARGET};

    BaseType_t xHigherPriorityTaskWoken;

    xQueueSendFromISR(rpm_queue, &set_target_event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int my_main() {
    rpm_queue = xQueueCreate(10, sizeof(Event));

    UserButton_Init(GPIO_MODE_IT_RISING);

    xTaskCreate(rpm_calc, "rpm", 300, NULL, 0, NULL);

    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

    vTaskStartScheduler();

    return 0;
}
