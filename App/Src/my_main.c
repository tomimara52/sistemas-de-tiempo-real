#include "FreeRTOS.h"
#include "button.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"
#include "tim.h"
#include "semphr.h"

#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494

#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_CS5 554 // C-sharp 5
#define NOTE_FS5 740 // F-sharp 5
#define REST 0// Convert frequency to ARR
uint32_t freq_to_arr(uint32_t freq)
{
    if (freq == 0) return 0;
    freq /= 2;
    return (1000000 / (16 * freq)) - 1;
}

const uint16_t melody[] = {
    NOTE_A4, NOTE_B4, NOTE_D5, NOTE_B4,
    NOTE_F5, NOTE_F5, NOTE_E5,

    NOTE_A4, NOTE_B4, NOTE_D5, NOTE_B4,
    NOTE_E5, NOTE_E5, NOTE_D5, 

    NOTE_A4, NOTE_B4, NOTE_D5, NOTE_B4,
    NOTE_D5, NOTE_E5, NOTE_C5, NOTE_A4, NOTE_A4, NOTE_E5, NOTE_D5
};

const uint16_t durations[] = {
    300, 300, 300, 300,
    600, 600, 600,

    300, 300, 300, 300,
    600, 600, 600,

    300, 300, 300, 300,
    900, 300, 600, 250, 300, 600, 300
};

void nevergiveyouup(void *argument)
{
    vTaskSuspend(NULL);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

    while (1)
    {
      for (uint32_t i = 0; i < sizeof(melody) / sizeof(uint16_t); ++i) {
        uint32_t freq = melody[i];

        if (freq == REST)
        {
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0); // silence
            vTaskDelay(pdMS_TO_TICKS(durations[i]));
        }
        else
        {
          uint32_t play_time = durations[i] * 0.85; // 85% sound
          uint32_t gap_time = durations[i] * 0.15;  // 15% silence
          uint32_t arr = freq_to_arr(freq);

          __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
          __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, arr / 2); // 50% duty
          vTaskDelay(pdMS_TO_TICKS(play_time));

          __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0); 
          vTaskDelay(pdMS_TO_TICKS(gap_time));
        }
      }
    }

        vTaskDelay(pdMS_TO_TICKS(1000)); // pause before repeating
}

SemaphoreHandle_t ngy_sem;

void nevergiveyouup_admin(void * void_handle)
{
  uint8_t ngy_running = 0;
  TaskHandle_t ngy_h = *((TaskHandle_t*) void_handle);

  for ( ;; ) {
    xSemaphoreTake(ngy_sem, portMAX_DELAY);

    if (ngy_running) {
      vTaskSuspend(ngy_h);
      ngy_running = 0;
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0); 
    } else {
      vTaskResume(ngy_h);
      ngy_running = 1;
    }
  }
}

void USER_BUTTON_Callback()
{
  BaseType_t higher_prio_task_awoken = pdFALSE;

  xSemaphoreGiveFromISR(ngy_sem, &higher_prio_task_awoken);

  portYIELD_FROM_ISR(higher_prio_task_awoken);
}


int my_main() {
    UserButton_Init(GPIO_MODE_IT_RISING);

    static TaskHandle_t ngy_handle;
    xTaskCreate(nevergiveyouup, "buzzy", 200, NULL, 0, &ngy_handle);
    xTaskCreate(nevergiveyouup_admin, "buzzy_admin", 100, (void*)&ngy_handle, 0, NULL);

    ngy_sem = xSemaphoreCreateBinary();

    vTaskStartScheduler();

    return 0;
}
