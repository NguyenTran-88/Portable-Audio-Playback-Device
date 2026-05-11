# MP3/Bluetooth Playback Device

STM32-based audio playback system with OLED user interface, button control, analog audio switching, and speaker output.

## Project Overview

This project implements a portable MP3/Bluetooth playback device using an STM32 microcontroller.  
The system supports music playback control, OLED display status, audio source switching, battery monitoring, and speaker amplification.

It was developed as an Embedded Systems course project by Group 2.

## Main Features

- MP3/Bluetooth audio playback
- OLED display user interface
- Button-based control interface
- External interrupt button handling
- Audio source switching circuit
- Battery voltage monitoring
- Speaker output using audio amplifier
- Layered firmware architecture using BSP modules

## Hardware Blocks

| Block | Main Components |
|---|---|
| Microcontroller | STM32F411CEU6 Blackpill |
| Display | OLED SSD1306 |
| Audio Playback | DFPlayer / MH-M28 Audio Module |
| Audio Switching | CD4053BE |
| Audio Amplifier | LM386 |
| Power Supply | LM2576T Buck Converter |
| User Input | Push Buttons |

## Firmware Architecture

The firmware is organized into multiple layers:

```text
Application Layer
│
├── BSP Layer
│   ├── bsp_button
│   ├── bsp_audio
│   ├── bsp_display
│   └── bsp_battery
│
├── Peripheral Drivers
│   ├── DFPlayer Driver
│   └── SSD1306 OLED Driver
│
└── STM32 HAL
```
## References
[1] https://github.com/afiskon/stm32-ssd1306
[2] https://github.com/nguyenkhue2608/STM32_Device_Lib
