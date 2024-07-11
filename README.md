# Dual-Core Microcontroller Oscilloscope and Signal Generator

## Project Overview

This project leverages a dual-core microcontroller to create an integrated oscilloscope and signal generator system with a touchscreen interface. The microcontroller is divided into two cores, each dedicated to specific functionalities to ensure smooth and efficient operation.

### Core Functionality

- **CM7 Core**:
  - Manages the oscilloscope functionality, capturing and plotting data in real-time.
  - Handles the user interface (UI) on the touchscreen, providing an intuitive and interactive experience.
  - Offers various oscilloscope features such as triggering and scaling.

- **CM4 Core**:
  - Operates the signal generator, allowing users to select and generate different waveforms.
  - Supports multiple waveform types including sine, square, triangle.

### Features

- **Touchscreen Interface**: 
  - User-friendly touchscreen for easy navigation and control.
  - Real-time waveform display and interactive oscilloscope controls.
  - Intuitive menus for selecting and configuring the signal generator parameters.

- **Oscilloscope**:
  - Real-time data acquisition and plotting.
  - Triggering options and adjustable scaling.

- **Signal Generator**:
  - Multiple waveform generation (sine, square, triangle, ecc.).
  
## Requirements

To use this project, you will need the following hardware:

- STM32H747I-DISCO Discovery kit
- Linear potentiometers

## Build and Run

``` shell
git clone git@github.com:Tonidotpy/oscilloscope.git
cd Makefile
make flash -j
```

## Project structure

```
Oscilloscope
├── CM4                                 # signal generator core
│   └── Core
│       ├── Inc
│       │   ├── main.h
│       │   ├── stm32h7xx_hal_conf.h
│       │   ├── stm32h7xx_it.h
│       │   └── waves.h                 # contains all the waves points
│       └── Src
│           ├── main.c
│           ├── stm32h7xx_hal_msp.c
│           ├── stm32h7xx_it.c
│           ├── syscalls.c
│           └── sysmem.c
├── CM7                                 # osscilloscope core
│   └── Core
│       ├── Inc
│       │   ├── chart_handler.h
│       │   ├── config.h
│       │   ├── lcd.h
│       │   ├── lvgl_api.h
│       │   ├── lvgl_colors.h
│       │   ├── main.h
│       │   ├── stm32h7xx_hal_conf.h
│       │   ├── stm32h7xx_it.h
│       │   ├── touch_screen.h
│       │   └── waves.h                 # contains all the waves points
│       └── Src
│           ├── chart_handler.c
│           ├── lcd.c
│           ├── lvgl_api.c
│           ├── main.c
│           ├── stm32h7xx_hal_msp.c
│           ├── stm32h7xx_it.c
│           ├── syscalls.c
│           ├── sysmem.c
│           └── touch_screen.c
├── Common
├── Drivers
│   ├── BSP
│   ├── CMSIS
│   └── STM32H7xx_HAL_Driver
├── Makefile
│   ├── CM4
│   ├── CM7
│   └── Makefile
├── README.md
├── openocd.cfg
└── oscilloscope.ioc
```