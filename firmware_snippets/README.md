# Firmware Snippets
This folder contains 22 selected firmware files from the ATxmega128A1 measurement platform.
The goal of this folder is to demonstrate the firmware structure, implementation style and selected technical concepts of the project without publishing the complete firmware source tree.
This repository is intended as a technical portfolio showcase. The files included here are selected parts of the original firmware and show representative implementation areas such as system startup, mode handling, peripheral drivers, display control, touch input, ADC acquisition, DMA configuration and spectrum rendering.
The complete firmware is not published in order to protect the original project work and hardware-specific implementation details.

---

## Included Files

firmware_snippets/
├── README.md
├── app_mode.c
├── app_mode.h
├── config.h
├── drv_adc.c
├── drv_adc.h
├── drv_clock.c
├── drv_clock.h
├── drv_dma_adc.c
├── drv_dma_adc.h
├── drv_ds1803.c
├── drv_ds1803.h
├── drv_gpio.c
├── drv_gpio.h
├── drv_lcd_ssd1289.c
├── drv_lcd_ssd1289.h
├── drv_touch_ads7843.c
├── drv_touch_ads7843.h
├── drv_usart.c
├── drv_usart.h
├── main.c
├── svc_spectrum_renderer.c
└── svc_spectrum_renderer.h

## File Overview

| File | Description |
|---|---|
| `main.c` | Contains the main application entry point, system initialization and high-level program flow. |
| `config.h` | Defines project-wide configuration values, constants and compile-time settings. |
| `app_mode.c` | Implements application-level mode handling and mode switching logic. |
| `app_mode.h` | Provides the public interface for application mode management. |
| `drv_adc.c` | Contains ADC configuration and low-level acquisition-related functions. |
| `drv_adc.h` | Provides the public ADC driver interface. |
| `drv_clock.c` | Implements system clock configuration for the ATxmega128A1 platform. |
| `drv_clock.h` | Provides the public interface for clock initialization and configuration. |
| `drv_dma_adc.c` | Implements DMA configuration for ADC-to-RAM sample transfer. |
| `drv_dma_adc.h` | Provides the public interface for the ADC DMA driver. |
| `drv_ds1803.c` | Implements control of the DS1803 digital potentiometer. |
| `drv_ds1803.h` | Provides the public interface for DS1803 configuration. |
| `drv_gpio.c` | Contains GPIO configuration and basic digital input/output handling. |
| `drv_gpio.h` | Provides the public GPIO driver interface. |
| `drv_lcd_ssd1289.c` | Implements the TFT display driver for the SSD1289 LCD controller. |
| `drv_lcd_ssd1289.h` | Provides the public interface for LCD initialization, drawing and display control. |
| `drv_touch_ads7843.c` | Implements touch controller handling for the ADS7843. |
| `drv_touch_ads7843.h` | Provides the public interface for touch input handling. |
| `drv_usart.c` | Contains USART communication functions used for debugging or serial communication. |
| `drv_usart.h` | Provides the public USART driver interface. |
| `svc_spectrum_renderer.c` | Implements spectrum visualization on the TFT display. |
| `svc_spectrum_renderer.h` | Provides the public interface for the spectrum renderer service. |


## Architecture Context

The selected files represent parts of the layered firmware architecture used in the project.

```text
Application Layer
    ↓
Service Layer
    ↓
Driver Layer
    ↓
Hardware
```

### Application Layer

The application layer manages the main program flow and user-facing mode logic.

Included files:

- `main.c`
- `app_mode.c`
- `app_mode.h`
- `config.h`

### Service Layer

The service layer contains higher-level functional logic such as rendering and signal-related processing.
Included files:

- `svc_spectrum_renderer.c`
- `svc_spectrum_renderer.h`

### Driver Layer

The driver layer provides low-level access to the ATxmega128A1 peripherals and external hardware components.
Included files:
- `drv_adc.c`
- `drv_adc.h`
- `drv_clock.c`
- `drv_clock.h`
- `drv_dma_adc.c`
- `drv_dma_adc.h`
- `drv_ds1803.c`
- `drv_ds1803.h`
- `drv_gpio.c`
- `drv_gpio.h`
- `drv_lcd_ssd1289.c`
- `drv_lcd_ssd1289.h`
- `drv_touch_ads7843.c`
- `drv_touch_ads7843.h`
- `drv_usart.c`
- `drv_usart.h`

---

## Technical Concepts Demonstrated

### System Startup and Mode Handling

The application files demonstrate how the embedded system is initialized and how different operating modes are managed.

Main concepts:

- Main application entry point
- System initialization sequence
- Mode-based execution
- Separation between application logic and hardware drivers

---

### Clock Configuration

The clock driver demonstrates how the system clock is configured for the ATxmega128A1.

Main concepts:

- Clock source configuration
- System timing preparation
- Peripheral timing basis

---

### ADC Configuration

The ADC driver demonstrates how analog signal acquisition is configured on the ATxmega128A1.

Main concepts:

- ADC initialization
- Input channel configuration
- Reference voltage selection
- Sampling configuration
- Register-based peripheral setup

---

### DMA-Based ADC Acquisition

The DMA ADC driver demonstrates how ADC samples are transferred directly into RAM without copying every sample manually through the CPU.

This is important for oscilloscope and spectrum analyzer operation because it reduces CPU load and improves acquisition timing.

Main concepts:

- DMA channel setup
- Source and destination configuration
- Transfer length configuration
- ADC result transfer to RAM
- Acquisition completion handling

---

### GPIO Control

The GPIO driver demonstrates basic digital input and output handling.

Main concepts:

- Pin direction configuration
- Digital output control
- Digital input reading
- Hardware abstraction for board-level signals

---

### DS1803 Digital Potentiometer Control

The DS1803 driver demonstrates the control of an external digital potentiometer used for analog signal scaling and adjustment.

Main concepts:

- Digital potentiometer configuration
- Hardware abstraction for analog adjustment
- Interface between firmware and analog front-end

---

### LCD Driver

The SSD1289 LCD driver demonstrates low-level TFT display control.

Main concepts:

- LCD initialization
- Register-level display configuration
- GRAM access
- Drawing primitives
- Pixel and area rendering
- Display output for measurement visualization

---

### Touch Controller Integration

The ADS7843 touch driver demonstrates how raw touch input is acquired and mapped to display coordinates.

Main concepts:

- Raw touch coordinate reading
- Touch filtering
- Calibration
- Screen coordinate mapping
- User input handling

---

### USART Communication

The USART driver provides serial communication support.

It can be used for:

- Debugging
- Development output
- Communication tests
- Diagnostic messages

---

### Spectrum Renderer

The spectrum renderer demonstrates how frequency-domain data is visualized on the TFT display.

Main concepts:

- Spectrum bar rendering
- Frequency axis display
- Magnitude scaling
- Peak visualization
- TFT-based graphical output

---

## What Is Not Included

The following parts are intentionally not included in this public showcase repository:

- Complete firmware source tree
- Complete build project
- Full oscilloscope implementation
- Full function generator implementation
- Complete FFT processing implementation
- Complete service layer
- Private calibration constants
- Complete hardware-specific configuration
- Internal project files
- Unredacted thesis documents

This protects the original project while still demonstrating the engineering work behind it.

## Review Guide
To understand the selected firmware structure, review the files in this order:

1. `main.c`
2. `config.h`
3. `app_mode.c` and `app_mode.h`
4. `drv_clock.c` and `drv_clock.h`
5. `drv_gpio.c` and `drv_gpio.h`
6. `drv_adc.c` and `drv_adc.h`
7. `drv_dma_adc.c` and `drv_dma_adc.h`
8. `drv_ds1803.c` and `drv_ds1803.h`
9. `drv_lcd_ssd1289.c` and `drv_lcd_ssd1289.h`
10. `drv_touch_ads7843.c` and `drv_touch_ads7843.h`
11. `drv_usart.c` and `drv_usart.h`
12. `svc_spectrum_renderer.c` and `svc_spectrum_renderer.h`

This order follows the logic from system startup to hardware abstraction, acquisition support, display control, user input and spectrum visualization.

## Important Note

These files are provided for portfolio and demonstration purposes only.

They are selected extracts from a larger firmware project and may depend on project-specific configuration files, macros, display functions, buffers or hardware definitions that are not included in this public repository.

They are not intended to represent a complete buildable firmware release.

---

## Relation to the Full Project

The full project integrates three operating modes:

- Digital Storage Oscilloscope
- FFT-based Spectrum Analyzer
- DDS-based Function Generator

The files in this folder mainly demonstrate the supporting firmware structure and selected hardware abstraction layers. They do not represent the entire implementation of the complete 3-in-1 measurement platform.
