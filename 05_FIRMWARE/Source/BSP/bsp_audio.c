/*
 * bsp_audio.c
 *
 * Board Support Package for the audio subsystem.
 *
 * Responsibilities:
 * - control DFPlayer Mini through the low-level DFPLAYER driver
 * - control Bluetooth module power enable pin
 * - control 74HC4053 audio source selection and inhibit pin
 * - maintain playback state for the display layer
 * - monitor DFPlayer BUSY pin and perform auto-next handling
 */


#include "bsp_audio.h"
#include "DFPLAYER.h"
#include <stdint.h>
#include <stddef.h>


// Volume parameter
#define BSP_AUDIO_DFP_VOL_MIN               0u
#define BSP_AUDIO_DFP_VOL_MAX               30u
#define BSP_AUDIO_DFP_VOL_INIT              20u


// Enable/disable external analog switch control
#define BSP_AUDIO_SOURCE_HW_SWITCH_EN       1u


// CD4053BE PINOUT
// SEL pin
#define BSP_AUDIO_SEL_GPIO_Port             GPIOB
#define BSP_AUDIO_SEL_Pin                   GPIO_PIN_3
#define BSP_AUDIO_SEL_SD_LEVEL              GPIO_PIN_RESET
#define BSP_AUDIO_SEL_BT_LEVEL              GPIO_PIN_SET

// Inh pin for muting
#define BSP_AUDIO_INH_GPIO_Port             GPIOB
#define BSP_AUDIO_INH_Pin                   GPIO_PIN_4
#define BSP_AUDIO_INH_ENABLE_LEVEL          GPIO_PIN_RESET
#define BSP_AUDIO_INH_DISABLE_LEVEL         GPIO_PIN_SET


// Bluetooth pin (only powered if the system is in bluetooth mode - through MOS and BJT control)
#define BSP_AUDIO_BT_EN_CONTROL_EN          1u
#define BSP_AUDIO_BT_EN_GPIO_Port           GPIOB
#define BSP_AUDIO_BT_EN_Pin                 GPIO_PIN_1
#define BSP_AUDIO_BT_EN_ACTIVE_LEVEL        GPIO_PIN_SET
#define BSP_AUDIO_BT_EN_INACTIVE_LEVEL      GPIO_PIN_RESET
#define BSP_AUDIO_BT_BOOT_WAIT_MS           300u


// DFPlayer BUSY pin. For most DFPlayer modules: LOW = playing, HIGH = idle
#define BSP_AUDIO_DFP_BUSY_GPIO_Port        GPIOB
#define BSP_AUDIO_DFP_BUSY_Pin              GPIO_PIN_5
#define BSP_AUDIO_DFP_BUSY_ACTIVE_LOW       1u
#define BSP_AUDIO_DFP_BUSY_STABLE_MS        20u
#define BSP_AUDIO_DFP_BUSY_IGNORE_CMD_MS    500u


// Startup and query timings
#define BSP_AUDIO_DFP_BOOT_WAIT_MS          2000u
#define BSP_AUDIO_DFP_SELECT_TF_WAIT_MS     300u
#define BSP_AUDIO_DFP_QUERY_TIMEOUT_MS      300u
#define BSP_AUDIO_DFP_NEXT_SETTLE_MS        250u
#define BSP_AUDIO_DFP_FALLBACK_MAX_TRACKS   255u


// Internal-initial control variables
static DFPLAYER_Name     g_dfp;
static uint8_t           g_is_playing       = 0u;
static uint8_t           g_volume           = BSP_AUDIO_DFP_VOL_INIT;
static BSP_AudioSource_t g_source           = BSP_AUDIO_SRC_DFPLAYER_SD;

// busy status for auto-next (can avoid unstable signal in initialization state)
static uint8_t           g_busy_idle_stable = 1u;
static uint8_t           g_busy_idle_sample = 1u;
static uint32_t          g_busy_tick        = 0u;
static uint32_t          g_dfp_cmd_tick     = 0u;

// Default track counting in case DFPLAYER failed init
static uint8_t           g_track            = 1u;
static uint8_t           g_track_total      = 5u;


// Audio control interface: Audio source, switch mute, power MH-M28, Auto next and Track count
static void    BSP_Audio_SetSource(BSP_AudioSource_t src);
static void    BSP_Audio_AudioSwitchMute(uint8_t mute);
static void    BSP_Audio_SetBtPower(uint8_t on);
static uint8_t BSP_Audio_DfpBusyIdleRaw(void);
static void    BSP_Audio_AutoNextByBusyService(void);
static uint8_t BSP_Audio_DetectTrackTotalByWrap(uint8_t *count_out);


// Init sequences
void BSP_Audio_Init(UART_HandleTypeDef *huart) {
	// Stick DFPLAYER and UART_1 together
    DFPLAYER_Init(&g_dfp, huart);

    // Off Bluetooth
    BSP_Audio_SetBtPower(0u);

    // Muting before configuring source to avoid pop, click noise
    BSP_Audio_AudioSwitchMute(1u);
    HAL_GPIO_WritePin(BSP_AUDIO_SEL_GPIO_Port,
                      BSP_AUDIO_SEL_Pin,
                      BSP_AUDIO_SEL_SD_LEVEL);

    // Allow DFPlayer internal boot and SD-card scan to complete
    HAL_Delay(BSP_AUDIO_DFP_BOOT_WAIT_MS);

    DFPLAYER_SetVolume(&g_dfp, g_volume);

    if (BSP_Audio_DetectTrackTotalByWrap(&g_track_total) == 0u) {
        g_track_total = 1u;
    }

    g_track = 1u;
    g_is_playing = 0u;


// Unmute Audio Path
    HAL_Delay(1u);
    BSP_Audio_AudioSwitchMute(0u);

    BSP_Audio_SetSource(BSP_AUDIO_SRC_DFPLAYER_SD);

    g_busy_idle_sample = BSP_Audio_DfpBusyIdleRaw();
    g_busy_idle_stable = g_busy_idle_sample;
    g_busy_tick        = HAL_GetTick();
    g_dfp_cmd_tick     = HAL_GetTick();
}



void BSP_Audio_Service(void) {
    BSP_Audio_AutoNextByBusyService();
}


// Play control: Ignore if in bluetooth
// Play -> Pause and vice versa
void BSP_Audio_PlayToggle(void) {
    if (g_source != BSP_AUDIO_SRC_DFPLAYER_SD) {
        return;
    }

    if (g_is_playing == 0u) {
        DFPLAYER_Play(&g_dfp);
        g_dfp_cmd_tick = HAL_GetTick();
        g_is_playing = 1u;
    }
    else {
        DFPLAYER_Pause(&g_dfp);
        g_dfp_cmd_tick = HAL_GetTick();
        g_is_playing = 0u;
    }
}


// Previous track: Auto play after switching track
void BSP_Audio_Prev(void) {
    if (g_source != BSP_AUDIO_SRC_DFPLAYER_SD) {
        return;
    }

    if (g_track_total == 0u) {
        g_track_total = 1u;
    }

    g_track = (g_track > 1u) ? (uint8_t)(g_track - 1u) : g_track_total;

    if (g_is_playing == 0u) {
        g_is_playing = 1u;
    }

    DFPLAYER_Prev(&g_dfp);
    g_dfp_cmd_tick = HAL_GetTick();
}


// Similar behavior as Prev
void BSP_Audio_Next(void) {
    if (g_source != BSP_AUDIO_SRC_DFPLAYER_SD) {
        return;
    }

    g_track = (g_track < g_track_total) ? (uint8_t)(g_track + 1u) : 1u;

    if (g_is_playing == 0u) {
        g_is_playing = 1u;
    }

    DFPLAYER_Next(&g_dfp);
    g_dfp_cmd_tick = HAL_GetTick();
}


void BSP_Audio_ModeToggle(void) {
    if (g_source == BSP_AUDIO_SRC_DFPLAYER_SD) {
        BSP_Audio_SetSource(BSP_AUDIO_SRC_BLUETOOTH);
    }
    else {
        BSP_Audio_SetSource(BSP_AUDIO_SRC_DFPLAYER_SD);
    }
}


void BSP_Audio_VolUp(void)
{
    if (g_source != BSP_AUDIO_SRC_DFPLAYER_SD) {
        return;
    }

    if (g_volume < BSP_AUDIO_DFP_VOL_MAX) {
        g_volume++;
        DFPLAYER_SetVolume(&g_dfp, g_volume);
    }
}

void BSP_Audio_VolDown(void) {
    if (g_source != BSP_AUDIO_SRC_DFPLAYER_SD) {
        return;
    }

    if (g_volume > BSP_AUDIO_DFP_VOL_MIN) {
        g_volume--;
        DFPLAYER_SetVolume(&g_dfp, g_volume);
    }
}


uint8_t BSP_Audio_IsPlaying(void) {
    return g_is_playing;
}


uint8_t BSP_Audio_GetVolume(void) {
    return g_volume;
}


uint8_t BSP_Audio_GetTrack(void) {
    return g_track;
}


uint8_t BSP_Audio_GetTrackTotal(void) {
    return g_track_total;
}


BSP_AudioSource_t BSP_Audio_GetSource(void) {
    return g_source;
}


static void BSP_Audio_SetSource(BSP_AudioSource_t src) {
	// Re-checking old status
    if (src == g_source) {
        return;
    }

    // Pause DFPlayer before leaving SD mode
    if ((src == BSP_AUDIO_SRC_BLUETOOTH) && (g_is_playing == 1u)) {
        DFPLAYER_Pause(&g_dfp);
        g_is_playing = 0u;
        g_dfp_cmd_tick = HAL_GetTick();
    }

    BSP_Audio_AudioSwitchMute(1u);

    if (src == BSP_AUDIO_SRC_BLUETOOTH) {
        BSP_Audio_SetBtPower(1u);
        HAL_Delay(BSP_AUDIO_BT_BOOT_WAIT_MS);
    }

    HAL_GPIO_WritePin(BSP_AUDIO_SEL_GPIO_Port,
                      BSP_AUDIO_SEL_Pin,
                      (src == BSP_AUDIO_SRC_DFPLAYER_SD) ? BSP_AUDIO_SEL_SD_LEVEL
                                                         : BSP_AUDIO_SEL_BT_LEVEL);
    HAL_Delay(1u);

    if (src == BSP_AUDIO_SRC_DFPLAYER_SD) {
        BSP_Audio_SetBtPower(0u);
    }
    BSP_Audio_AudioSwitchMute(0u);

    g_source = src;
}


// Sample Busy pin state - LOW = PLAYING
static uint8_t BSP_Audio_DfpBusyIdleRaw(void) {
    GPIO_PinState busy_pin = HAL_GPIO_ReadPin(BSP_AUDIO_DFP_BUSY_GPIO_Port,
                                              BSP_AUDIO_DFP_BUSY_Pin);
    return (busy_pin == GPIO_PIN_SET) ? 1u : 0u;
}


// Auto-next when DFPLAYER is not busy ((except being controlled)
static void BSP_Audio_AutoNextByBusyService(void) {
    uint32_t now;
    uint8_t idle_now;
    uint8_t prev_idle;

    if (g_source != BSP_AUDIO_SRC_DFPLAYER_SD) {
        return;
    }

    now = HAL_GetTick();

    // Ignore noise when DFPLAYER just receive cmd
    if ((now - g_dfp_cmd_tick) < BSP_AUDIO_DFP_BUSY_IGNORE_CMD_MS) {
        return;
    }

    idle_now = BSP_Audio_DfpBusyIdleRaw();

    if (idle_now != g_busy_idle_sample) {
        g_busy_idle_sample = idle_now;
        g_busy_tick = now;
        return;
    }

    // Same behavior as debouncing, busy pin can change randomly
    if ((now - g_busy_tick) < BSP_AUDIO_DFP_BUSY_STABLE_MS) {
        return;
    }

    // Stable means the idle state is accepted
    if (g_busy_idle_stable != g_busy_idle_sample) {
        prev_idle = g_busy_idle_stable;
        g_busy_idle_stable = g_busy_idle_sample;

        // Triggers: playing (0u) -> idle (1u) one more is it's playing g_is_playing = 1
        if ((prev_idle == 0u) && (g_busy_idle_stable == 1u)) {
            if (g_is_playing != 0u) {
                DFPLAYER_Next(&g_dfp);
                g_is_playing = 1u;
                g_track = (g_track < g_track_total) ? (uint8_t)(g_track + 1u) : 1u;
                g_dfp_cmd_tick = now;
            }
        }
    }
}


static uint8_t BSP_Audio_DetectTrackTotalByWrap(uint8_t *count_out) {
    uint16_t start_track = 0u;
    uint16_t cur_track   = 0u;
    uint16_t step        = 0u;

    // Avoid system fault when return track - POINTER =/= NULL
    if (count_out == NULL) {
        return 0u;
    }

    *count_out = 1u;

    DFPLAYER_SelectTF(&g_dfp);
    HAL_Delay(BSP_AUDIO_DFP_SELECT_TF_WAIT_MS);

    // Save first track index
    if (DFPLAYER_QueryCurrentTfTrack(&g_dfp,
                                     &start_track,
                                     BSP_AUDIO_DFP_QUERY_TIMEOUT_MS) == 0u) {
        return 0u;
    }

    // Validate returned start track
    if ((start_track == 0u) || (start_track > BSP_AUDIO_DFP_FALLBACK_MAX_TRACKS)) {
        return 0u;
    }

    // Assign track = step after routing back to track 1st
    for (step = 1u; step <= BSP_AUDIO_DFP_FALLBACK_MAX_TRACKS; step++) {
        DFPLAYER_Next(&g_dfp);
        HAL_Delay(BSP_AUDIO_DFP_NEXT_SETTLE_MS);

        if (DFPLAYER_QueryCurrentTfTrack(&g_dfp,
                                         &cur_track,
                                         BSP_AUDIO_DFP_QUERY_TIMEOUT_MS) == 0u) {
            return 0u;
        }

        if (cur_track == start_track) {
            *count_out = (uint8_t)step;
            DFPLAYER_SelectTF(&g_dfp);
            HAL_Delay(BSP_AUDIO_DFP_SELECT_TF_WAIT_MS);
            return 1u;
        }
    }

    DFPLAYER_SelectTF(&g_dfp);
    HAL_Delay(BSP_AUDIO_DFP_SELECT_TF_WAIT_MS);

    return 0u;
}


static void BSP_Audio_AudioSwitchMute(uint8_t mute) {
    HAL_GPIO_WritePin(BSP_AUDIO_INH_GPIO_Port,
                      BSP_AUDIO_INH_Pin,
                      (mute != 0u) ? BSP_AUDIO_INH_DISABLE_LEVEL
                                   : BSP_AUDIO_INH_ENABLE_LEVEL);
}


static void BSP_Audio_SetBtPower(uint8_t on) {
    HAL_GPIO_WritePin(BSP_AUDIO_BT_EN_GPIO_Port,
                      BSP_AUDIO_BT_EN_Pin,
                      (on != 0u) ? BSP_AUDIO_BT_EN_ACTIVE_LEVEL
                                 : BSP_AUDIO_BT_EN_INACTIVE_LEVEL);
}
