# Technical Decisions
This document summarizes key technical decisions made during the development of the ATxmega128A1 measurement platform.

## 1. Mode-Based System Architecture
The system does not run oscilloscope, spectrum analyzer and function generator simultaneously.
Instead, it uses a mode-based architecture.

Reason:
- Avoids timer conflicts
- Avoids DMA conflicts
- Reduces SRAM pressure
- Simplifies system control
- Improves stability

## 2. DMA-Based ADC Acquisition
ADC samples are transferred to RAM using DMA instead of CPU polling.
Reason:
- Reduces CPU load
- Improves acquisition timing
- Allows high-speed sampling
- Keeps sampling independent from display rendering

## 3. 8-Bit ADC Storage
Although the ADC supports higher resolution, the oscilloscope stores samples as 8-bit values.

Reason:
- Reduces memory usage
- Allows 4096 samples in a 4 KB buffer
- Improves record length
- Supports waveform visualization within SRAM limits

## 4. Software Triggering
The trigger is implemented in software after the ADC samples are captured.

Reason:
- Uses the same data that is displayed
- Avoids a separate analog trigger path
- Simplifies hardware
- Allows edge detection with hysteresis

## 5. FFT Reuse of ADC Buffer
The spectrum analyzer reuses the captured oscilloscope buffer for FFT processing.

Reason:
- Avoids a second large buffer
- Reduces SRAM usage
- Makes FFT processing feasible on the ATxmega128A1

## 6. DMA-Based Waveform Output
Sine and triangle signals are output through lookup tables and DMA.

Reason:
- Reduces CPU load
- Provides regular DAC updates
- Allows the CPU to handle UI tasks

## 7. ISR-Based Square/PWM Output
Square and PWM signals are generated using a timer interrupt.

Reason:
- Only two output levels are required
- Duty cycle can be controlled efficiently
- Implementation remains simple and deterministic

## 8. Public Showcase Instead of Full Source Release
The public repository does not include the complete firmware implementation.

Reason:
- Protects the original project work
- Prevents direct copying
- Still demonstrates software architecture and technical competence
- Keeps the repository focused as a recruiter portfolio