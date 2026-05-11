/*
 * bsp_display.c
 *
 * Board Support Package for OLED UI rendering.
 *
 * Responsibilities:
 * - initialize SSD1306 OLED display
 * - render MP3 playback screen, Bluetooth screen, track, volume, and battery
 * - reduce OLED I2C traffic by updating only changed regions
 * - animate the playback wave while MP3 playback is active
 *
 * Low-level OLED communication remains inside ssd1306.c.
 * Bitmap data remains inside ui_assets.c.
 */


#include "bsp_display.h"
#include "bsp_audio.h"
#include "bsp_battery.h"
#include "ui_assets.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"


#include <stdio.h>
#include <stdint.h>


#define BSP_DISPLAY_TRACK_X              54u
#define BSP_DISPLAY_TRACK_Y               6u
#define BSP_DISPLAY_TRACK_W              80u
#define BSP_DISPLAY_TRACK_H              10u


#define BSP_DISPLAY_VOL_X                23u
#define BSP_DISPLAY_VOL_Y                27u
#define BSP_DISPLAY_VOL_W                12u
#define BSP_DISPLAY_VOL_H                 8u


#define BSP_DISPLAY_BAT_X                 9u
#define BSP_DISPLAY_BAT_Y                49u
#define BSP_DISPLAY_BAT_W                26u
#define BSP_DISPLAY_BAT_H                 8u


#define BSP_DISPLAY_WAVE_ANIM_PERIOD_MS 300u


static uint8_t  g_wave_frame     = 0u;
static uint32_t g_wave_anim_tick = 0u;


static uint8_t           g_cache_valid       = 0u;
static BSP_AudioSource_t g_cache_source      = BSP_AUDIO_SRC_DFPLAYER_SD;
static uint8_t           g_cache_is_playing  = 0u;
static uint8_t           g_cache_track       = 1u;
static uint8_t           g_cache_track_total = 1u;
static uint8_t           g_cache_volume      = 0u;
static uint8_t           g_cache_battery     = 255u;

static void BSP_Display_DrawTrack(void);
static void BSP_Display_DrawVolume(void);
static void BSP_Display_DrawBattery(void);
static void BSP_Display_UpdateCacheFromSystem(void);


void BSP_Display_Init(void) {
    ssd1306_Init();
    BSP_Display_ResetWaveAnim();
    g_cache_valid = 0u;
}


//Draw first screen
void BSP_Display_FirstDraw(void) {
    BSP_Display_DrawPauseFull();
    BSP_Display_DrawTrack();
    BSP_Display_DrawVolume();
    BSP_Display_DrawBattery();

    BSP_Display_UpdateCacheFromSystem();
    g_cache_valid = 1u;
}


// Redraw the full UI when source or play/pause state changes
void BSP_Display_RefreshAll(void) {
    BSP_AudioSource_t src     = BSP_Audio_GetSource();
    uint8_t           playing = BSP_Audio_IsPlaying();

    if (src == BSP_AUDIO_SRC_BLUETOOTH) {
        BSP_Display_DrawBluetoothFull();
        BSP_Display_DrawBattery();
    }
    else {
        if (g_cache_valid == 0u) {
            BSP_Display_DrawPauseFull();
        }
        else if (g_cache_source != BSP_AUDIO_SRC_DFPLAYER_SD) {
            BSP_Display_DrawPauseFull();
        }
        else if ((g_cache_is_playing == 0u) && (playing != 0u)) {
            BSP_Display_DrawPlayIconOnly();
        }
        else if ((g_cache_is_playing != 0u) && (playing == 0u)) {
            BSP_Display_DrawPauseIconOnly();
        }

        BSP_Display_DrawTrack();
        BSP_Display_DrawVolume();
        BSP_Display_DrawBattery();
    }

    BSP_Display_UpdateCacheFromSystem();
    g_cache_valid = 1u;
}


// Reset wave animation to the first frame and restart animation timing
void BSP_Display_ResetWaveAnim(void) {
    g_wave_frame = 0u;
    g_wave_anim_tick = HAL_GetTick();
}


// Periodically refresh only changed UI fields and animate wave while playing
void BSP_Display_Service(void) {
    BSP_AudioSource_t src         = BSP_Audio_GetSource();
    uint8_t           playing     = BSP_Audio_IsPlaying();
    uint8_t           track       = BSP_Audio_GetTrack();
    uint8_t           track_total = BSP_Audio_GetTrackTotal();
    uint8_t           volume      = BSP_Audio_GetVolume();
    uint8_t           battery     = BSP_Battery_GetPercent();
    uint32_t          now;

    if ((g_cache_valid == 0u) ||
        (src != g_cache_source) ||
        (playing != g_cache_is_playing)) {
        BSP_Display_RefreshAll();
        return;
    }

    if (src == BSP_AUDIO_SRC_DFPLAYER_SD) {
        if ((track != g_cache_track) || (track_total != g_cache_track_total)) {
            BSP_Display_DrawTrack();
            g_cache_track = track;
            g_cache_track_total = track_total;
        }

        if (volume != g_cache_volume) {
            BSP_Display_DrawVolume();
            g_cache_volume = volume;
        }

        if (battery != g_cache_battery) {
            BSP_Display_DrawBattery();
            g_cache_battery = battery;
        }
    }
    else {
        if (battery != g_cache_battery) {
            BSP_Display_DrawBattery();
            g_cache_battery = battery;
        }
    }

    if ((src != BSP_AUDIO_SRC_DFPLAYER_SD) || (playing == 0u)) {
        return;
    }

    now = HAL_GetTick();
    if ((now - g_wave_anim_tick) < BSP_DISPLAY_WAVE_ANIM_PERIOD_MS) {
        return;
    }

    g_wave_anim_tick = now;

    BSP_Display_DrawWaveFrame(g_wave_frame);
    BSP_Display_DrawTrack();
    BSP_Display_DrawVolume();

    g_wave_frame++;
    if (g_wave_frame >= 10u) {
        g_wave_frame = 0u;
    }

    BSP_Display_UpdateCacheFromSystem();
}

void BSP_Display_DrawPauseFull(void) {
    ssd1306_Fill(Black);
    ssd1306_DrawBitmap(0, 0, ui_pause_bitmap, 128, 64, White);
    ssd1306_UpdateScreen();
}

void BSP_Display_DrawPauseIconOnly(void) {
    ssd1306_Fill(Black);
    ssd1306_DrawBitmap(0, 0, ui_pause_bitmap, 128, 64, White);
    ssd1306_UpdateArea(70, 42, 28, 28);
}

void BSP_Display_DrawPlayIconOnly(void) {
    ssd1306_Fill(Black);
    ssd1306_DrawBitmap(0, 0, ui_play_bitmap, 128, 64, White);
    ssd1306_UpdateArea(70, 42, 28, 28);
}

void BSP_Display_DrawBluetoothFull(void) {
    ssd1306_Fill(Black);
    ssd1306_DrawBitmap(0, 0, ui_bluetooth_bitmap, 128, 64, White);
    ssd1306_UpdateScreen();
}

void BSP_Display_DrawWaveFrame(uint8_t frame) {
    const uint8_t *wave_frames[] = {
        ui_wave_7,
        ui_wave_9,
        ui_wave_5,
        ui_wave_1,
        ui_wave_6,
        ui_wave_4,
        ui_wave_2,
        ui_wave_8,
        ui_wave_3,
        ui_wave_10,
    };

    const uint8_t *bmp = wave_frames[frame % 10u];

    ssd1306_Fill(Black);
    ssd1306_DrawBitmap(0, 0, bmp, 128, 64, White);
    ssd1306_UpdateArea(54, 18, 62, 18);
}


static void BSP_Display_DrawTrack(void) {
    char s[16];

    snprintf(s, sizeof(s), "Track: %u /%u",
             (unsigned int)BSP_Audio_GetTrack(),
             (unsigned int)BSP_Audio_GetTrackTotal());

    ssd1306_FillRectangle(BSP_DISPLAY_TRACK_X,
                          BSP_DISPLAY_TRACK_Y,
                          BSP_DISPLAY_TRACK_X + BSP_DISPLAY_TRACK_W - 1u,
                          BSP_DISPLAY_TRACK_Y + BSP_DISPLAY_TRACK_H - 1u,
                          Black);

    ssd1306_SetCursor(BSP_DISPLAY_TRACK_X, BSP_DISPLAY_TRACK_Y);
    ssd1306_WriteString(s, Font_6x8, White);

    ssd1306_UpdateArea(BSP_DISPLAY_TRACK_X,
                       BSP_DISPLAY_TRACK_Y,
                       BSP_DISPLAY_TRACK_W,
                       BSP_DISPLAY_TRACK_H);
}


static void BSP_Display_DrawVolume(void) {
    char s[5];

    snprintf(s, sizeof(s), "%u", (unsigned int)BSP_Audio_GetVolume());

    ssd1306_FillRectangle(BSP_DISPLAY_VOL_X,
                          BSP_DISPLAY_VOL_Y,
                          BSP_DISPLAY_VOL_X + BSP_DISPLAY_VOL_W - 1u,
                          BSP_DISPLAY_VOL_Y + BSP_DISPLAY_VOL_H - 1u,
                          Black);

    ssd1306_SetCursor(BSP_DISPLAY_VOL_X, BSP_DISPLAY_VOL_Y);
    ssd1306_WriteString(s, Font_6x8, White);

    ssd1306_UpdateArea(BSP_DISPLAY_VOL_X,
                       BSP_DISPLAY_VOL_Y,
                       BSP_DISPLAY_VOL_W,
                       BSP_DISPLAY_VOL_H);
}


static void BSP_Display_DrawBattery(void) {
    char s[12];

    snprintf(s, sizeof(s), "%u%%", (unsigned int)BSP_Battery_GetPercent());

    ssd1306_FillRectangle(BSP_DISPLAY_BAT_X,
                          BSP_DISPLAY_BAT_Y,
                          BSP_DISPLAY_BAT_X + BSP_DISPLAY_BAT_W - 1u,
                          BSP_DISPLAY_BAT_Y + BSP_DISPLAY_BAT_H - 1u,
                          Black);

    ssd1306_SetCursor(BSP_DISPLAY_BAT_X, BSP_DISPLAY_BAT_Y);
    ssd1306_WriteString(s, Font_6x8, White);

    ssd1306_UpdateArea(BSP_DISPLAY_BAT_X,
                       BSP_DISPLAY_BAT_Y,
                       BSP_DISPLAY_BAT_W,
                       BSP_DISPLAY_BAT_H);
}

// Store current audio and battery states into display cache
static void BSP_Display_UpdateCacheFromSystem(void) {
    g_cache_source      = BSP_Audio_GetSource();
    g_cache_is_playing  = BSP_Audio_IsPlaying();
    g_cache_track       = BSP_Audio_GetTrack();
    g_cache_track_total = BSP_Audio_GetTrackTotal();
    g_cache_volume      = BSP_Audio_GetVolume();
    g_cache_battery     = BSP_Battery_GetPercent();
}
