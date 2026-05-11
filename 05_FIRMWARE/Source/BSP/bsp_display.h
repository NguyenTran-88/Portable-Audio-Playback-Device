#ifndef BSP_DISPLAY_H_
#define BSP_DISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "main.h"
#include <stdint.h>


void BSP_Display_Init(void);
void BSP_Display_FirstDraw(void);
void BSP_Display_Service(void);
void BSP_Display_RefreshAll(void);
void BSP_Display_ResetWaveAnim(void);


void BSP_Display_DrawPauseFull(void);
void BSP_Display_DrawPauseIconOnly(void);
void BSP_Display_DrawPlayIconOnly(void);
void BSP_Display_DrawBluetoothFull(void);
void BSP_Display_DrawWaveFrame(uint8_t frame);

#ifdef __cplusplus
}
#endif

#endif /* BSP_DISPLAY_H_ */
