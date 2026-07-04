# Firmware Folder Structure
The original firmware was organized into a layered architecture.

firmware/
│
├── main.c
├── config.h
│
├── app/
│   ├── app_menu.c
│   ├── app_menu.h
│   ├── app_siggen.c
│   └── app_siggen.h
│
├── drivers/
│   ├── drv_adc.c
│   ├── drv_adc.h
│   ├── drv_dac.c
│   ├── drv_dac.h
│   ├── drv_dma_adc.c
│   ├── drv_dma_adc.h
│   ├── drv_dma_dac.c
│   ├── drv_dma_dac.h
│   ├── drv_timer_sample.c
│   ├── drv_timer_sample.h
│   ├── drv_timer_gen.c
│   ├── drv_timer_gen.h
│   ├── drv_touch_ads7843.c
│   ├── drv_touch_ads7843.h
│   ├── drv_ds1803.c
│   └── drv_ds1803.h
│
└── services/
    ├── svc_acq.c
    ├── svc_acq.h
    ├── svc_trigger.c
    ├── svc_trigger.h
    ├── svc_renderer.c
    ├── svc_renderer.h
    ├── svc_freq_fft.c
    ├── svc_freq_fft.h
    ├── svc_spectrum_renderer.c
    ├── svc_spectrum_renderer.h
    ├── svc_siggen.c
    └── svc_siggen.h
	
Design Reason
The structure separates:
Hardware access
Signal processing
Rendering
User interface
Mode control

This improves readability, testability and maintainability.