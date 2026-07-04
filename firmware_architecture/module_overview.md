# Module Overview

| Layer       | Module   | Responsibility |
|---|---|---|
| Application | `main.c` | System initialization, mode selection and main control flow |
| Application | `app_menu| Start menu, touch selection and mode navigation |
| Application | `app_siggen` | Function generator user interface |
| Driver      | `drv_adc`| ADC configuration for signal acquisition |
| Driver      | `drv_dac`| DAC configuration for waveform output |
| Driver      | `drv_dma_adc` | DMA transfer from ADC to RAM |
| Driver      | `drv_dma_dac` | DMA transfer from waveform table to DAC |
| Driver      | `drv_timer_sample` | Sampling timer for oscilloscope and spectrum analyzer |
| Driver      | `drv_timer_gen` | DAC update timer for function generator |
| Driver      | `drv_touch_ads7843` | Touchscreen coordinate acquisition and filtering |
| Driver      | `drv_ds1803` | DS1803 digital potentiometer control |
| Service     | `svc_acq` | Acquisition buffer management |
| Service     | `svc_trigger` | Software edge trigger with hysteresis |
| Service     | `svc_renderer` | Oscilloscope waveform rendering |
| Service     | `svc_freq_fft` | FFT preprocessing, transformation and peak detection |
| Service     | `svc_spectrum_renderer` | Spectrum visualization on TFT |
| Service     | `svc_siggen` | Waveform generation and signal generator configuration |