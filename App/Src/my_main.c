#include "FreeRTOS.h"
#include "button.h"
#include "queue.h"
#include "stdbool.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"
#include "tim.h"
#include "timers.h"

// timer clock: 16Mhz
// prescaler: 15
// => frequency: 16Mhz / 16 = 1Mhz
const uint32_t TIMER_FREQ = 1000000;
const float TIMER_FREQ_FLOAT = (float)TIMER_FREQ;

const uint32_t MAX_PERIOD_MS = 6000;

// have to be more than THRESH_PERCENT% away from target_rpm for buzzer to sound
const uint32_t THRESH_PERCENT = 10;
const float THRESH_LOWER = 1.0f - ((float)THRESH_PERCENT / 100.0f);
const float THRESH_UPPER = 1.0f + ((float)THRESH_PERCENT / 100.0f);

const float ALPHA = 0.3f;

const uint32_t PULSE = 500;

const uint32_t BASE_BEEP_PERIOD = 750;
const uint32_t MIN_BEEP_PERIOD = 100;
const uint32_t RPM_BEEP_STEP = 5;

// Assuming your default CubeMX ARR for TIM3 is 1000
const uint32_t BASE_ARR = 1000; 
const uint32_t PITCH_STEP = 5;       // How much the pitch changes per RPM deviance
const uint32_t MIN_ARR = 200;        // Upper limit for pitch (preventing ultrasonic/overflow)
const uint32_t MAX_ARR = 4000;       // Lower limit for pitch (preventing clicking noises)

enum Button {
    BUTTON_SET,
    BUTTON_RESET,
};

enum State {
    STOPPED,
    RUNNING,
    RUNNING_TARGET_SET,
    STOPPED_TARGET_SET,
};

QueueHandle_t ir_queue = NULL;

QueueHandle_t rpm_queue_to_fsm = NULL;
QueueHandle_t button_queue_to_fsm = NULL;
QueueSetHandle_t fsm_set = NULL;

QueueHandle_t target_queue_to_buzzer = NULL;
QueueHandle_t rpm_queue_to_buzzer = NULL;
QueueSetHandle_t buzzer_set = NULL;

TimerHandle_t buzzer_timer = NULL;
uint32_t buzzer_timer_period = BASE_BEEP_PERIOD;
uint32_t buzzer_arr = BASE_ARR;

void buzzer_timer_cb(TimerHandle_t xTimer) {
    uint32_t dynamic_pulse = buzzer_arr / 2;

    __HAL_TIM_SET_AUTORELOAD(&htim3, buzzer_arr);
    __HAL_TIM_SET_COMPARE(
        &htim3, TIM_CHANNEL_2,
        __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_2) > 0 ? 0 : dynamic_pulse);

    xTimerChangePeriod(xTimer, buzzer_timer_period, 0);
}

uint8_t update_buzzer(float rpm, float target, uint8_t buzzer_buzzing) {
    float diff = rpm < target ? target - rpm : rpm - target;

    // 1. Handle Beep Cadence (Interval between beeps)
    buzzer_timer_period =
        (diff * RPM_BEEP_STEP < BASE_BEEP_PERIOD - MIN_BEEP_PERIOD)
            ? BASE_BEEP_PERIOD - diff * RPM_BEEP_STEP
            : MIN_BEEP_PERIOD;

    // 2. Handle Audio Pitch (Frequency of the tone itself)
    uint32_t new_arr = BASE_ARR;
    uint32_t change = (uint32_t)(diff * PITCH_STEP);

    if (rpm > target) {
        // Going too fast -> Higher Pitch -> Lower ARR
        new_arr = (BASE_ARR > change + MIN_ARR) ? BASE_ARR - change : MIN_ARR;
    } else {
        // Going too slow -> Lower Pitch -> Higher ARR
        new_arr = BASE_ARR + change;
        if (new_arr > MAX_ARR) {
            new_arr = MAX_ARR;
        }
    }

    buzzer_arr = new_arr;

    // 3. Manage FreeRTOS software timer state
    if (!buzzer_buzzing) {
        xTimerChangePeriod(buzzer_timer, buzzer_timer_period, portMAX_DELAY);
        buzzer_buzzing = true;
    }

    return buzzer_buzzing;
}  

void buzzer(void *args) {
    float target = 0.0f;
    float rpm = 0.0f;

    uint8_t buzzer_buzzing = false;

    for (;;) {
        TickType_t ms_to_wait =
            target == 0.0f ? portMAX_DELAY : (60.0f / (target * THRESH_LOWER)) * 1000;
        QueueSetMemberHandle_t selected =
            xQueueSelectFromSet(buzzer_set, pdMS_TO_TICKS(ms_to_wait));
        if (selected != NULL) {
            if (selected == rpm_queue_to_buzzer) {
                xQueueReceive(rpm_queue_to_buzzer, &rpm, 0);
            } else if (selected == target_queue_to_buzzer) {
                xQueueReceive(target_queue_to_buzzer, &target, 0);
            }

            if (target != 0.0f &&
                (rpm < target * THRESH_LOWER || rpm > target * THRESH_UPPER)) {
                buzzer_buzzing = update_buzzer(rpm, target, buzzer_buzzing);

            } else {
                xTimerStop(buzzer_timer, portMAX_DELAY);
                __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
                buzzer_arr = BASE_ARR;
                buzzer_buzzing = false;
            }
        } else {
            buzzer_buzzing = update_buzzer(rpm, target, buzzer_buzzing);
        }
    }
}

void fsm(void *args) {
    enum State state = STOPPED;
    float target_rpm = 0.0f;
    float rpm = 0.0f;

    for (;;) {
        QueueSetMemberHandle_t selected =
            xQueueSelectFromSet(fsm_set, portMAX_DELAY);

        if (selected == rpm_queue_to_fsm) {
            xQueueReceive(rpm_queue_to_fsm, &rpm, 0);

            if (state == RUNNING && rpm == 0.0f) {
                state = STOPPED;
            } else if (state == RUNNING_TARGET_SET && rpm == 0.0f) {
                state = STOPPED_TARGET_SET;
            } else if (state == STOPPED && rpm > 0.0f) {
                state = RUNNING;
            } else if (state == STOPPED_TARGET_SET && rpm > 0.0f) {
                state = RUNNING_TARGET_SET;
            }
        } else if (selected == button_queue_to_fsm) {
            enum Button button;
            xQueueReceive(button_queue_to_fsm, &button, 0);

            switch (button) {
            case BUTTON_SET:
                target_rpm = rpm;
                if (state == RUNNING)
                    state = RUNNING_TARGET_SET;
                xQueueOverwrite(target_queue_to_buzzer, &target_rpm);
                break;
            case BUTTON_RESET:
                break;
            }
        }

        switch (state) {
        case STOPPED:
            printf("stopped\t");
            break;
        case RUNNING:
            printf("running\t");
            break;
        case RUNNING_TARGET_SET:
            printf("running target set\t");
            break;
        case STOPPED_TARGET_SET:
            printf("stopped target set\t");
            break;
        }
        printf("rpm: %d.%02d\t", (int)rpm, ((int)(rpm * 100)) % 100);
        printf("target rpm: %d.%02d\n", (int)target_rpm,
               ((int)(target_rpm * 100)) % 100);
    }
}

void rpm_calc(void *args) {
    uint32_t prev_capture = 0;
    uint32_t period = 0;

    for (;;) {
        uint32_t new_capture;
        float rpm = 0.0f;

        if (xQueueReceive(ir_queue, &new_capture,
                          pdMS_TO_TICKS(MAX_PERIOD_MS)) == pdPASS) {

            period =
                ALPHA * (new_capture - prev_capture) + (1.0f - ALPHA) * period;
            rpm = (TIMER_FREQ_FLOAT / period) * 60.0f;

            prev_capture = new_capture;
        }

        xQueueOverwrite(rpm_queue_to_fsm, &rpm);
        xQueueOverwrite(rpm_queue_to_buzzer, &rpm);
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

    enum Button button = BUTTON_SET;
    xQueueSendFromISR(button_queue_to_fsm, &button, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int my_main() {
    ir_queue = xQueueCreate(10, sizeof(uint32_t));

    button_queue_to_fsm = xQueueCreate(10, sizeof(enum Button));
    rpm_queue_to_fsm = xQueueCreate(1, sizeof(float));
    fsm_set = xQueueCreateSet(11);
    xQueueAddToSet(rpm_queue_to_fsm, fsm_set);
    xQueueAddToSet(button_queue_to_fsm, fsm_set);

    target_queue_to_buzzer = xQueueCreate(1, sizeof(float));
    rpm_queue_to_buzzer = xQueueCreate(1, sizeof(float));
    buzzer_set = xQueueCreateSet(2);
    xQueueAddToSet(target_queue_to_buzzer, buzzer_set);
    xQueueAddToSet(rpm_queue_to_buzzer, buzzer_set);

    UserButton_Init(GPIO_MODE_IT_RISING);

    xTaskCreate(rpm_calc, "rpm", 300, NULL, 0, NULL);
    xTaskCreate(fsm, "fsm", 300, NULL, 0, NULL);
    xTaskCreate(buzzer, "buzzer", 300, NULL, 0, NULL);

    buzzer_timer = xTimerCreate("buz", 100, pdTRUE, 0, buzzer_timer_cb);

    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

    vTaskStartScheduler();

    return 0;
}
