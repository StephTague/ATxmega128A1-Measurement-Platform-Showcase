# Application Layer
The application layer connects the user interface with the system functions.
It is responsible for:
- System startup
- Start menu display
- Touch-based mode selection
- Mode-specific control flow
- User parameter handling

## Start Menu
The start menu allows the user to select one of three modes:
- Oscilloscope
- Spectrum Analyzer
- Function Generator

## Mode-Based Execution
Only one mode is active at a time.
This avoids conflicts between:
- ADC and DAC operation
- DMA channels
- Timers
- RAM buffers
- TFT display updates

## Function Generator UI
The function generator UI allows the user to adjust:
- Waveform
- Frequency
- Vpp
- Offset
- Duty cycle
- Output state

## Design Goal
The application layer keeps user interaction separate from low-level hardware access and signal-processing code.