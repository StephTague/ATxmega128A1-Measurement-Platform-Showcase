# Architecture Overview
The measurement platform uses a mode-based architecture. Only one operating mode is active at a time.
This approach avoids resource conflicts between:
- ADC
- DAC
- DMA channels
- Timers
- SRAM
- TFT display access
- Touch interface

## System Flow
System Start
    ↓
Hardware Initialization
    ↓
Start Menu
    ↓
Mode Selection
    ↓
Oscilloscope Mode / Spectrum Analyzer Mode / Function Generator Mode

## Layered Firmware Architecture
The firmware is divided into three layers.

### Driver Layer
The driver layer provides direct access to hardware peripherals and external components.
Examples:
ADC
DAC
DMA
Timers
TFT display
Touch controller
DS1803 digital potentiometer

### Service Layer
The service layer implements signal acquisition, processing and rendering logic.
Examples:
Acquisition service
Trigger service
FFT service
Oscilloscope renderer
Spectrum renderer
Signal generator service

### Application Layer
The application layer manages the user interface and the selected operating mode.
Examples:
Start menu
Mode selection
Oscilloscope application flow
Spectrum analyzer application flow
Function generator interface

### Resource Sharing
The oscilloscope and spectrum analyzer share the ADC/DMA acquisition path.
The function generator uses the DAC output path and is kept separate from the measurement modes.
This design keeps the system simple, robust and suitable for a resource-limited microcontroller.