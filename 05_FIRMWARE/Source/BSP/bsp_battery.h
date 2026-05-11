#ifndef BSP_BATTERY_H_
#define BSP_BATTERY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

void    BSP_Battery_Init(ADC_HandleTypeDef *hadc);
void    BSP_Battery_Service(void);
float   BSP_Battery_GetVoltage(void);
uint8_t BSP_Battery_GetPercent(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BATTERY_H_ */
