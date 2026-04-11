#include "FreeRTOS.h"
#include "button.h"
#include "stm32h5xx_nucleo.h"
#include "task.h"

void blinky(void *args) {
    for (;;) {
        BSP_LED_Toggle(LED_GREEN);

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

void USER_BUTTON_Callback() {
    printf("hola\n");
}

int my_main() {
    UserButton_Init(GPIO_MODE_IT_RISING);
    xTaskCreate(blinky, "blinky", 100, NULL, 0, NULL);
    vTaskStartScheduler();

    return 0;
}
