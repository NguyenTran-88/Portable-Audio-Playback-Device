/*
 * app.c
 *
 * Application super-loop for the system
 */

#include "app_main.h"
#include "bsp_button.h"
#include "bsp_audio.h"
#include "bsp_display.h"
#include "bsp_battery.h"

static void App_HandleEvent(BSP_ButtonEvent_t evt);

void App_Init(UART_HandleTypeDef *huart, ADC_HandleTypeDef *hadc) {
    BSP_Button_Init();
    BSP_Audio_Init(huart);

    HAL_Delay(1000u);

    BSP_Battery_Init(hadc);
    BSP_Display_Init();
    BSP_Display_FirstDraw();
}

void App_Loop(void) {
    BSP_ButtonEvent_t evt;

    BSP_Audio_Service();
    BSP_Battery_Service();

    while (BSP_Button_PopEvent(&evt) != 0u) {
        App_HandleEvent(evt);
    }

    BSP_Display_Service();
}

static void App_HandleEvent(BSP_ButtonEvent_t evt) {
    switch (evt)
    {
        case BSP_BUTTON_EVT_PLAY_TOGGLE:
            BSP_Audio_PlayToggle();
            BSP_Display_ResetWaveAnim();
            BSP_Display_RefreshAll();
            break;

        case BSP_BUTTON_EVT_PREV_TRACK:
            BSP_Audio_Prev();
            BSP_Display_ResetWaveAnim();
            BSP_Display_RefreshAll();
            break;

        case BSP_BUTTON_EVT_NEXT_TRACK:
            BSP_Audio_Next();
            BSP_Display_ResetWaveAnim();
            BSP_Display_RefreshAll();
            break;

        case BSP_BUTTON_EVT_MODE_TOGGLE:
            BSP_Audio_ModeToggle();
            BSP_Display_ResetWaveAnim();
            BSP_Display_RefreshAll();
            break;

        case BSP_BUTTON_EVT_VOL_DOWN:
            BSP_Audio_VolDown();
            BSP_Display_RefreshAll();
            break;

        case BSP_BUTTON_EVT_VOL_UP:
            BSP_Audio_VolUp();
            BSP_Display_RefreshAll();
            break;

        case BSP_BUTTON_EVT_NONE:
        default:
            break;
    }
}
