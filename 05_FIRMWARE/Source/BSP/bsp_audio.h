#ifndef BSP_AUDIO_H_
#define BSP_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

// Struct define
typedef enum {
    BSP_AUDIO_SRC_DFPLAYER_SD = 0,
    BSP_AUDIO_SRC_BLUETOOTH
} BSP_AudioSource_t;


void BSP_Audio_Init(UART_HandleTypeDef *huart);
void BSP_Audio_Service(void);


// Control APIs
void BSP_Audio_PlayToggle(void);
void BSP_Audio_Prev(void);
void BSP_Audio_Next(void);
void BSP_Audio_ModeToggle(void);
void BSP_Audio_VolUp(void);
void BSP_Audio_VolDown(void);


// Status-return APIs
uint8_t           BSP_Audio_IsPlaying(void);
uint8_t           BSP_Audio_GetVolume(void);
uint8_t           BSP_Audio_GetTrack(void);
uint8_t           BSP_Audio_GetTrackTotal(void);
BSP_AudioSource_t BSP_Audio_GetSource(void);


#ifdef __cplusplus
}
#endif

#endif /* BSP_AUDIO_H_ */
