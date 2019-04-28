#ifndef INIT_MAIN_H
#define INIT_MAIN_H

#include "stm32f4xx_hal.h"

void SystemClock_Config(void);
void MX_USART3_UART_Init(void);
void MX_GPIO_Init(void);
void MX_FSMC_Init(void);

void Error_Handler(void);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

#endif //INIT_MAIN_H
