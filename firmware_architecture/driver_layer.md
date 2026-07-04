# Driver Layer
The driver layer contains low-level hardware access functions.
It is responsible for configuring and controlling the ATxmega128A1 peripherals and external components.

## Main Drivers

### ADC Driver
Responsible for:
- ADC configuration
- Reference selection
- Input channel configuration
- Conversion mode setup

### DAC Driver
Responsible for:
- DAC initialization
- DAC channel configuration
- Output value update

### DMA Drivers
Responsible for:
- ADC-to-RAM transfer
- RAM-to-DAC transfer
- DMA channel configuration
- Transfer completion handling

### Timer Drivers
Responsible for:
- ADC sampling time base
- DAC update time base
- UI timing

### Touch Driver
Responsible for:
- SPI communication
- ADS7843 raw coordinate reading
- Coordinate filtering
- Mapping raw values to screen coordinates

### DS1803 Driver
Responsible for:
- I2C/TWI communication
- Wiper configuration
- Analog front-end and output-stage adjustment

## Design Goal
The driver layer hides hardware register details from the service and application layers.