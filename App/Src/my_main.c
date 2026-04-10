#include "FreeRTOS.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"

void blinky(void *args) {
  for (;;) {
    BSP_LED_Toggle(LED_GREEN);

    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

int my_main() {
  xTaskCreate(blinky, "blinky", 100, NULL, 0, NULL);
  vTaskStartScheduler();

  return 0;
}
