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
const uint32_t MAX_PERIOD_MS = 6000;

// have to be more than THRESH_PERCENT% away from target_rpm for buzzer to sound
const uint32_t THRESH_PERCENT = 10;
// period is inverse of rpm so limits are upside down
const float PERIOD_THRESH_UPPER = 100.0f / (100.0f - (float)THRESH_PERCENT);
const float PERIOD_THRESH_LOWER = 100.0f / (100.0f + (float)THRESH_PERCENT);

const float ALPHA = 0.3f;

enum Button {
    BUTTON_SET,
    BUTTON_RESET,
};

enum State {
    STOPPED,
    RUNNING,
    TARGET_SET,
};

QueueHandle_t ir_queue = NULL;
QueueHandle_t rpm_queue = NULL;
QueueHandle_t button_queue = NULL;
QueueSetHandle_t fsm_set = NULL;

void fsm(void* args) {
    enum State state = STOPPED;

    for (;;) {
         QueueSetMemberHandle_t selected = xQueueSelectFromSet(fsm_set, portMAX_DELAY);
        
        if (selected == rpm_queue) {
            float rpm;
            xQueueReceive(rpm_queue, &rpm, 0);

            if (rpm == 0.0f) {
                state = STOPPED;
                printf("stopped\t");
            } else if (state == STOPPED && rpm > 0.0f) {
                state = RUNNING;
            }
            printf("rpm: %d.%02d\n", (int)rpm, ((int)(rpm * 100)) % 100);
        } else if (selected == button_queue) {
        }
    }
}


void rpm_calc(void *args) {
    uint32_t prev_capture = 0;
    uint32_t period = 0;

    for (;;) {
        uint32_t new_capture;
        float rpm = 0.0f;

        if (xQueueReceive(ir_queue, &new_capture, pdMS_TO_TICKS(MAX_PERIOD_MS)) == pdPASS) {

            period = ALPHA * (new_capture - prev_capture) + (1.0f - ALPHA) * period;
            rpm = (TIMER_FREQ_FLOAT / period) * 60.0f;

            prev_capture = new_capture;

        } 

        xQueueOverwrite(rpm_queue, &rpm);
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        uint32_t capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        BaseType_t xHigherPriorityTaskWoken;

        xQueueSendFromISR(ir_queue, &capture, &xHigherPriorityTaskWoken);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void USER_BUTTON_Callback() {

    BaseType_t xHigherPriorityTaskWoken;

    //xQueueSendFromISR(rpm_queue, &set_target_event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int my_main() {
    ir_queue = xQueueCreate(10, sizeof(uint32_t));
    rpm_queue = xQueueCreate(1, sizeof(float));

    fsm_set = xQueueCreateSet(1);
    xQueueAddToSet(rpm_queue, fsm_set);

    UserButton_Init(GPIO_MODE_IT_RISING);

    xTaskCreate(rpm_calc, "rpm", 300, NULL, 0, NULL);
    xTaskCreate(fsm, "fsm", 300, NULL, 0, NULL);

    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

    vTaskStartScheduler();

    return 0;
}
