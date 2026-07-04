# ATxmega128A1 Measurement Platform Showcase
![Embedded C](https://img.shields.io/badge/Embedded%20C-Bare--Metal-blue)
![Microcontroller](https://img.shields.io/badge/MCU-ATxmega128A1-green)
![ADC](https://img.shields.io/badge/ADC-DMA%20Acquisition-orange)
![FFT](https://img.shields.io/badge/DSP-FFT-purple)
![DDS](https://img.shields.io/badge/Signal%20Generation-DDS-red)
![Status](https://img.shields.io/badge/Status-Prototype%20Validated-success)

This repository presents a technical showcase of the project: a compact embedded measurement platform based on the ATxmega128A1 microcontroller.
The system integrates three operating modes on one embedded platform:
- Digital Storage Oscilloscope
- FFT-based Spectrum Analyzer
- DDS-based Function Generator
This repository is intended as a portfolio showcase for  engineering review. It is not a complete open-source firmware release.

## Project Overview
The goal of this project was to design and implement a compact 3-in-1 embedded measurement platform on a resource-limited microcontroller.
The platform combines signal acquisition, signal analysis and signal generation in one system. It uses the internal ADC, DAC, timers, DMA controller, XMEGA event system, TFT display and touch interface of the ATxmega128A1 platform.
The application is mode-based. After startup, the user selects one of the available modes from a touch-based start menu. Only one mode is active at a time, which avoids resource conflicts between ADC, DAC, DMA, timers, SRAM and TFT display access.

## Key Features

### Digital Storage Oscilloscope
- Timer-triggered ADC sampling
- DMA-based data acquisition
- 4096-sample acquisition buffer
- Software edge trigger with hysteresis
- Min-max waveform rendering
- TFT-based signal visualization
- Display of Vmin, Vmax, Vpp and frequency

### FFT-based Spectrum Analyzer
- Reuse of the oscilloscope ADC/DMA acquisition path
- 1024-point FFT
- Decimated ADC data
- DC removal
- Optional Hann windowing
- Peak detection
- Spectrum display on TFT

### DDS-based Function Generator
- Sine, square/PWM and triangle waveform generation
- DAC-based signal output
- Timer-controlled update rate
- ISR-based square/PWM generation
- DMA-based sine and triangle output
- Adjustable frequency, Vpp, offset and duty cycle
- DS1803-controlled analog output stage

## Tech Stack
- Bare-metal C
- ATxmega128A1
- ADC / DAC
- DMA
- Eventsys
- I2C / SPI / USART
- Timers
- XMEGA Event System
- Interrupt Service Routines
- FFT-based signal processing
- DDS-based waveform generation
- TFT display rendering
- Touch interface
- DS1803 digital potentiometer
- Analog front-end electronics

## Software Architecture
The firmware follows a layered architecture:
Driver Layer: low-level hardware access
Service Layer: signal acquisition, signal processing and rendering
Application Layer: user interface, mode control and system flow
More details are available in:
firmware_architecture/

## Source Code Availability
This repository is intended as a technical portfolio showcase.
It contains selected firmware snippets, architecture documentation, diagrams, screenshots and validation results.
The complete firmware source code is not published in order to protect the original project work and hardware-specific implementation details.
The provided snippets demonstrate the main technical concepts, but they are not intended to represent a complete reusable firmware implementation.

## How To review the project:
This repository is not intended to be cloned and built as a complete firmware release.
1. Read this README for the system overview.
2. Open the documents in docs/.
3. Review the firmware structure in firmware_architecture/.
4. Check the screenshots and watch the demo videos  in Medias/.
5. Review selected implementation snippets in firmware_snippets/.

## What I Learned
This project helped me improve my understanding of:
Bare-metal embedded firmware design
Hardware/software integration
Efficient use of limited SRAM
DMA-based data acquisition
Timer-based deterministic sampling
ADC and DAC configuration
FFT processing on microcontrollers
DDS waveform generation
TFT display rendering
Touch-based user interaction
Modular firmware architecture

## Project Status
The prototype was implemented and tested as part of a master thesis.
The system successfully demonstrates the integration of digital storage oscilloscope functionality, FFT-based spectrum analysis and DDS-based function generation on a single ATxmega128A1-based embedded platform.