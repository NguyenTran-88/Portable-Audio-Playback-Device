#ifndef BSP_BUTTON_H_
#define BSP_BUTTON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>


typedef enum
{
    BSP_BUTTON_EVT_NONE = 0,
    BSP_BUTTON_EVT_PLAY_TOGGLE,
    BSP_BUTTON_EVT_PREV_TRACK,
    BSP_BUTTON_EVT_NEXT_TRACK,
    BSP_BUTTON_EVT_VOL_DOWN,
    BSP_BUTTON_EVT_VOL_UP,
    BSP_BUTTON_EVT_MODE_TOGGLE
} BSP_ButtonEvent_t;


void    BSP_Button_Init(void);
uint8_t BSP_Button_PopEvent(BSP_ButtonEvent_t *evt_out);


#ifdef __cplusplus
}
#endif

#endif /* BSP_BUTTON_H_ */
