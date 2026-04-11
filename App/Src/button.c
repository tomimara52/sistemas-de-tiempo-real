#include "stm32h5xx_nucleo.h"

__weak void USER_BUTTON_Callback() {}

void UserButton_Init() {
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitTypeDef gpio_init_structure;
  gpio_init_structure.Pin = GPIO_PIN_13;
  gpio_init_structure.Pull = GPIO_PULLDOWN;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio_init_structure.Mode = GPIO_MODE_IT_RISING;

  HAL_GPIO_Init(GPIOC, &gpio_init_structure);

  (void)HAL_EXTI_GetHandle(hpb_exti, BUTTON_USER_EXTI_LINE);
  (void)HAL_EXTI_RegisterCallback(hpb_exti, HAL_EXTI_COMMON_CB_ID,
                                  USER_BUTTON_Callback);

  HAL_NVIC_SetPriority(BUTTON_USER_EXTI_IRQ, BSP_BUTTON_USER_IT_PRIORITY, 0x00);
  HAL_NVIC_EnableIRQ(BUTTON_USER_EXTI_IRQ);
}
