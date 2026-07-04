This folder contains selected hardware-related documentation for the measurement platform.
The complete hardware design is not published. Only selected diagrams and redacted schematics are included to explain the system architecture.
## Main Hardware Blocks
- ATxmega128A1 microcontroller
- Analog front-end for oscilloscope input
- ADC signal acquisition path
- DAC output path for function generation
- DS1803 digital potentiometer
- TFT display with SSD1289 controller
- ADS7843 touch controller
- Operational amplifier stages for signal conditioning

## Included Material
hardware/
│
├── README.md
│
├── block_diagrams/
│   ├── oscilloscope_signal_flow.png
│   ├── spectrum_analyzer_signal_flow.png
│   ├── function_generator_signal_flow.png
│   └── system_integration_overview.png
│
└── selected_schematics_redacted/
    ├── analog_frontend_redacted.png
    ├── tft_touch_connection_redacted.png
    └── function_generator_output_stage_redacted.png
Note
The schematics are included only for demonstration and portfolio purposes.
They are not intended to be used as production-ready hardware design files.