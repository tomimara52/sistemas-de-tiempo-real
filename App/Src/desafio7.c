#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32h5xx_nucleo.h"

typedef struct _blinkyArg {
    SemaphoreHandle_t next_sem;
    SemaphoreHandle_t my_sem;
    GPIO_TypeDef *my_port;
    uint16_t my_pin;
    GPIO_TypeDef *prev_port;
    uint16_t prev_pin;
} BlinkyArg;

void blinky_sem(void *arg_void) {
    BlinkyArg arg = *((BlinkyArg *)arg_void);

    for (;;) {
        xSemaphoreTake(arg.my_sem, portMAX_DELAY);

        HAL_GPIO_WritePin(arg.prev_port, arg.prev_pin, 0);
        HAL_GPIO_WritePin(arg.my_port, arg.my_pin, 1);

        vTaskDelay(pdMS_TO_TICKS(100));

        xSemaphoreGive(arg.next_sem);
    }
}


void desafio7(void) {
    static SemaphoreHandle_t sem_red;
    static SemaphoreHandle_t sem_green;
    static SemaphoreHandle_t sem_blue;

    sem_red = xSemaphoreCreateBinary();
    sem_green = xSemaphoreCreateBinary();
    sem_blue = xSemaphoreCreateBinary();

    static BlinkyArg red_arg;
    red_arg.my_sem = sem_red;
    red_arg.next_sem = sem_green;
    red_arg.my_port = GPIOB;
    red_arg.my_pin = GPIO_PIN_13;
    red_arg.prev_port = GPIOB;
    red_arg.prev_pin = GPIO_PIN_15;
    xTaskCreate(blinky_sem, "red", 100, (void*)&red_arg, 0, NULL);

    static BlinkyArg green_arg;
    green_arg.my_sem = sem_green;
    green_arg.next_sem = sem_blue;
    green_arg.my_port = GPIOB;
    green_arg.my_pin = GPIO_PIN_14;
    green_arg.prev_port = GPIOB;
    green_arg.prev_pin = GPIO_PIN_13;
    xTaskCreate(blinky_sem, "green", 100, (void*)&green_arg, 0, NULL);

    static BlinkyArg blue_arg;
    blue_arg.my_sem = sem_blue;
    blue_arg.next_sem = sem_red;
    blue_arg.my_port = GPIOB;
    blue_arg.my_pin = GPIO_PIN_15;
    blue_arg.prev_port = GPIOB;
    blue_arg.prev_pin = GPIO_PIN_14;
    xTaskCreate(blinky_sem, "blue", 100, (void*)&blue_arg, 0, NULL);

    xSemaphoreGive(sem_red);
}
