# Project Summary
This project is an embedded measurement platform based on the ATxmega128A1 microcontroller.
It combines three instruments in one compact system:
- Digital Storage Oscilloscope
- FFT-based Spectrum Analyzer
- DDS-based Function Generator

The goal was to demonstrate that a resource-limited 8-bit microcontroller can be used to implement signal acquisition, signal analysis and signal generation in a single embedded platform.

## Main System Functions

### Oscilloscope
The oscilloscope captures analog input signals through an analog front-end and stores the digitized samples in RAM using DMA.
Main characteristics:
- 8-bit ADC storage
- Up to 2 MS/s sampling rate
- 4096-sample acquisition buffer
- Software edge trigger
- TFT waveform display

### Spectrum Analyzer
The spectrum analyzer reuses the oscilloscope acquisition path. After a complete ADC frame is captured, the signal is processed using FFT.
Main characteristics:
- 1024-point FFT
- Decimated ADC samples
- DC removal
- Hann windowing
- Peak detection
- Spectrum visualization

### Function Generator
The function generator produces test signals through the DAC and an analog output stage.
Supported waveforms:
- Sine
- Square / PWM
- Triangle

Main characteristics:
- DAC-based output
- Timer-controlled update rate
- ISR-based square/PWM generation
- DMA-based sine and triangle output
- Adjustable frequency, amplitude and offset

## Engineering Focus
This project demonstrates:
- Bare-metal C programming
- Peripheral-level microcontroller programming
- ADC/DAC usage
- DMA-based data transfer
- Timer/event-system synchronization
- FFT-based signal analysis
- DDS-based signal generation
- TFT rendering
- Touch-based user interaction
- Modular embedded software architecture