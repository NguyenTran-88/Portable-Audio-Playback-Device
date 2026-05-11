/*
 * bsp_battery.c
 *
 * Board Support Package for battery monitoring.
 *
 * Responsibilities:
 * - read the divided 3-cell Li-ion battery voltage through ADC
 * - convert raw ADC value to pack voltage
 * - estimate battery percentage for the OLED UI
 * - apply a light low-pass filter to stabilize the displayed value
 */


#include "bsp_battery.h"
#include <stddef.h>


#define BSP_BAT_R_TOP_OHM             68000.0f
#define BSP_BAT_R_BOT_OHM             20000.0f


#define BSP_BAT_ADC_VREF                  3.3f
#define BSP_BAT_ADC_MAX                4095.0f


// 3-cell Li-ion display range
#define BSP_BAT_FULL_V                  12.6f
#define BSP_BAT_EMPTY_V                  9.0f


#define BSP_BAT_UPDATE_PERIOD_MS        2000u
#define BSP_BAT_ADC_SAMPLES              16u
#define BSP_BAT_CAL_GAIN              1.000f


static ADC_HandleTypeDef *g_hadc = NULL;

static float   g_battery_voltage = 0.0f;
static uint8_t g_battery_percent = 0u;
static uint32_t g_last_update_ms = 0u;

static uint16_t BSP_Battery_ReadRawAverage(uint8_t samples);
static float   BSP_Battery_RawToVoltage(uint16_t raw);
static uint8_t BSP_Battery_VoltageToPercent(float vbat);


void BSP_Battery_Init(ADC_HandleTypeDef *hadc) {
    uint16_t raw;
    g_hadc = hadc;

    raw = BSP_Battery_ReadRawAverage(BSP_BAT_ADC_SAMPLES);
    g_battery_voltage = BSP_Battery_RawToVoltage(raw);
    g_battery_percent = BSP_Battery_VoltageToPercent(g_battery_voltage);
    g_last_update_ms = HAL_GetTick();
}


void BSP_Battery_Service(void) {
    uint16_t raw;
    float new_voltage;
    uint32_t now = HAL_GetTick();

    if ((now - g_last_update_ms) < BSP_BAT_UPDATE_PERIOD_MS) {
        return;
    }

    g_last_update_ms = now;

    raw = BSP_Battery_ReadRawAverage(BSP_BAT_ADC_SAMPLES);
    new_voltage = BSP_Battery_RawToVoltage(raw);

    if (g_battery_voltage < 0.1f)  {
        g_battery_voltage = new_voltage;
    }
    // Exponential Moving Average
    else {
        g_battery_voltage = (0.85f * g_battery_voltage) + (0.15f * new_voltage);
    }

    g_battery_percent = BSP_Battery_VoltageToPercent(g_battery_voltage);
}


float BSP_Battery_GetVoltage(void) {
    return g_battery_voltage;
}


uint8_t BSP_Battery_GetPercent(void) {
    return g_battery_percent;
}


// Start ADC
// Wait conversion
// Read ADC value if conversion OK
// Stop ADC
static uint16_t BSP_Battery_ReadRawAverage(uint8_t samples){
    uint32_t sum = 0u;
    uint8_t i;

    if ((g_hadc == NULL) || (samples == 0u)) {
        return 0u;
    }

    for (i = 0u; i < samples; i++) {
        if (HAL_ADC_Start(g_hadc) != HAL_OK) {
            continue;
        }

        if (HAL_ADC_PollForConversion(g_hadc, 10u) == HAL_OK) {
            sum += HAL_ADC_GetValue(g_hadc);
        }

        HAL_ADC_Stop(g_hadc);
    }

    return (uint16_t)(sum / samples);
}


static float BSP_Battery_RawToVoltage(uint16_t raw) {
    float v_adc;
    float v_bat;

    v_adc = ((float)raw * BSP_BAT_ADC_VREF) / BSP_BAT_ADC_MAX;
    v_bat = v_adc * ((BSP_BAT_R_TOP_OHM + BSP_BAT_R_BOT_OHM) / BSP_BAT_R_BOT_OHM);

    return v_bat * BSP_BAT_CAL_GAIN;
}


static uint8_t BSP_Battery_VoltageToPercent(float vbat) {
    float percent;

    if (vbat <= BSP_BAT_EMPTY_V) {
        return 0u;
    }

    if (vbat >= BSP_BAT_FULL_V) {
        return 100u;
    }

    percent = ((vbat - BSP_BAT_EMPTY_V) * 100.0f) / (BSP_BAT_FULL_V - BSP_BAT_EMPTY_V);

    if (percent < 0.0f) {
        percent = 0.0f;
    }
    else if (percent > 100.0f) {
        percent = 100.0f;
    }

    return (uint8_t)(percent + 0.5f);
}
