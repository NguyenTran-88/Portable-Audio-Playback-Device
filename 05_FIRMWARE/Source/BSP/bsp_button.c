/*
 * bsp_button.c
 *
 * Board Support Package for user buttons.
 *
 * Responsibilities:
 * - map GPIO EXTI interrupts to high-level button events
 * - debounce button presses
 * - push events from ISR context into a lightweight ring buffer
 * - allow the application super-loop to pop button events safely
 */


#include "bsp_button.h"
#include <stddef.h>


#define BSP_BUTTON_DEBOUNCE_MS      200u
#define BSP_BUTTON_QUEUE_SIZE       16u


// Struct storing data type of BSP_ButtonMap_t: port, pin, event, last time recorded
typedef struct	{
    GPIO_TypeDef      *port;
    uint16_t           pin;
    BSP_ButtonEvent_t  evt;
    uint32_t           last_irq_tick;
} BSP_ButtonMap_t;



// Current board mapping.
// GPIO_MODE_IT_FALLING + GPIO_PULLUP.

static BSP_ButtonMap_t g_buttons[] = {
    { GPIOA, GPIO_PIN_3,  BSP_BUTTON_EVT_PLAY_TOGGLE, 0u }, // PA3  : Play/Pause
    { GPIOA, GPIO_PIN_4,  BSP_BUTTON_EVT_PREV_TRACK,  0u }, // PA4  : Previous
    { GPIOA, GPIO_PIN_5,  BSP_BUTTON_EVT_NEXT_TRACK,  0u }, // PA5  : Next
    { GPIOA, GPIO_PIN_8,  BSP_BUTTON_EVT_VOL_DOWN,    0u }, // PA8  : Volume Down
    { GPIOA, GPIO_PIN_9,  BSP_BUTTON_EVT_VOL_UP,      0u }, // PA9  : Volume Up
    { GPIOA, GPIO_PIN_10, BSP_BUTTON_EVT_MODE_TOGGLE, 0u }, // PA10 : Mode
};


// Buffer ring declaration & index pointer
static volatile uint8_t g_evt_q[BSP_BUTTON_QUEUE_SIZE];
static volatile uint8_t g_evt_q_head = 0u;
static volatile uint8_t g_evt_q_tail = 0u;


// last interrupt call & numbers of call
static volatile uint16_t g_dbg_last_exti_pin = 0u;
static volatile uint32_t g_dbg_exti_count[16] = {0u};


//
static int32_t BSP_Button_FindIndexByPin(uint16_t pin);
static void    BSP_Button_PostEventFromISR(BSP_ButtonEvent_t evt);


// Reset all variables and buffer
// (disable and enable Interrupt Handler in the process)
void BSP_Button_Init(void) {
    uint32_t i;
    __disable_irq();

    g_evt_q_head = 0u;
    g_evt_q_tail = 0u;
    g_dbg_last_exti_pin = 0u;

    for (i = 0u; i < BSP_BUTTON_QUEUE_SIZE; i++) {
        g_evt_q[i] = 0u;
    }

    for (i = 0u; i < 16u; i++) {
        g_dbg_exti_count[i] = 0u;
    }

    for (i = 0u; i < (uint32_t)(sizeof(g_buttons) / sizeof(g_buttons[0])); i++) {
        g_buttons[i].last_irq_tick = 0u;
    }
    __enable_irq();
}


// Function return flag and event when being called by app_main
// Concurrency protection: prevent event from being cleared while executing another
uint8_t BSP_Button_PopEvent(BSP_ButtonEvent_t *evt_out) {
    uint8_t has_data = 0u;

    if (evt_out == NULL) {
        return 0u;
    }

// tail != head => events being popped
// tail = head after popping
    __disable_irq();
    if (g_evt_q_tail != g_evt_q_head) {
        *evt_out = (BSP_ButtonEvent_t)g_evt_q[g_evt_q_tail];
        g_evt_q_tail = (uint8_t)((g_evt_q_tail + 1u) % BSP_BUTTON_QUEUE_SIZE);
        has_data = 1u;
    }
    __enable_irq();

    return has_data;
}


// HAL EXTI callback (STM32 call this instead the one in stm32f4xx_it.c)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    int32_t  idx;
    uint32_t now = HAL_GetTick();

    g_dbg_last_exti_pin = GPIO_Pin;
    idx = BSP_Button_FindIndexByPin(GPIO_Pin); // Finding GPIO number

    if (idx < 0) {
        return;
    }

    switch (GPIO_Pin) {
        case GPIO_PIN_3:  g_dbg_exti_count[3]++;  break;
        case GPIO_PIN_4:  g_dbg_exti_count[4]++;  break;
        case GPIO_PIN_5:  g_dbg_exti_count[5]++;  break;
        case GPIO_PIN_8:  g_dbg_exti_count[8]++;  break;
        case GPIO_PIN_9:  g_dbg_exti_count[9]++;  break;
        case GPIO_PIN_10: g_dbg_exti_count[10]++; break;
        default: break;
    }

    // Debouncing: take only first edge - ignore the others until 200ms
    if ((now - g_buttons[(uint32_t)idx].last_irq_tick) < BSP_BUTTON_DEBOUNCE_MS) {
        return;
    }
    g_buttons[(uint32_t)idx].last_irq_tick = now;
    BSP_Button_PostEventFromISR(g_buttons[(uint32_t)idx].evt);
}


static int32_t BSP_Button_FindIndexByPin(uint16_t pin)
{
    uint32_t i;

    for (i = 0u; i < (uint32_t)(sizeof(g_buttons) / sizeof(g_buttons[0])); i++)
    {
        if (g_buttons[i].pin == pin)
        {
            return (int32_t)i;
        }
    }

    return -1;
}


// Push event into buffer, increase head index
static void BSP_Button_PostEventFromISR(BSP_ButtonEvent_t evt)
{
    uint8_t next = (uint8_t)((g_evt_q_head + 1u) % BSP_BUTTON_QUEUE_SIZE);

    // If the queue is full, the new event is dropped intentionally
    if (next != g_evt_q_tail)
    {
        g_evt_q[g_evt_q_head] = (uint8_t)evt;
        g_evt_q_head = next;
    }
}


uint16_t BSP_Button_GetLastExtiPin(void) {
    return g_dbg_last_exti_pin;
}


uint32_t BSP_Button_GetExtiCount(uint8_t pin_index) {
    if (pin_index >= 16u) {
        return 0u;
    }
    return g_dbg_exti_count[pin_index];
}
