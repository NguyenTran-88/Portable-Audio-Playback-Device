#ifndef APP_H_
#define APP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void App_Init(UART_HandleTypeDef *huart, ADC_HandleTypeDef *hadc);
void App_Loop(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_H_ */
