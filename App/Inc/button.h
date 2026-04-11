#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdint.h>

void UserButton_Init(uint32_t gpio_mode);
void USER_BUTTON_Callback(void);

#endif
