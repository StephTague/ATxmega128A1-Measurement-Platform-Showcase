# Service Layer
The service layer contains the main functional logic of the measurement platform.
It uses the driver layer but does not directly manage user interface screens.

## Acquisition Service
Responsible for:
- Managing the ADC sample buffer
- Starting and stopping acquisition
- Checking acquisition completion
- Providing access to captured samples

## Trigger Service
Responsible for:
- Software edge detection
- Rising and falling edge search
- Trigger hysteresis
- Trigger index calculation

## Oscilloscope Renderer
Responsible for:
- Mapping ADC values to display coordinates
- Min-max rendering
- Drawing waveform data
- Calculating Vmin, Vmax, Vpp and frequency

## FFT Service
Responsible for:
- Preparing ADC samples for FFT
- Decimation
- DC removal
- Windowing
- FFT execution
- Magnitude calculation
- Peak detection

## Spectrum Renderer
Responsible for:
- Drawing spectrum bars
- Scaling magnitudes
- Displaying frequency peaks
- Drawing frequency axis and labels

## Signal Generator Service
Responsible for:
- Configuring waveform parameters
- Calculating DAC codes
- Generating square/PWM signals
- Building sine and triangle tables
- Managing ISR or DMA output mode