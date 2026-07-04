# Selected Firmware Interfaces
This document shows selected firmware interfaces to demonstrate the structure of the project.
The complete implementation is intentionally not included in this public showcase repository.

/* Acquisition service */
void svc_acq_init(void);
void svc_acq_start(void);
uint8_t svc_acq_done(void);
uint8_t *svc_acq_get_ch0_raw(void);

/* Trigger service */
int16_t svc_trigger_find_edge_near_u8(const uint8_t *buf,
                                      uint16_t start,
                                      uint16_t end,
                                      uint8_t level_raw,
                                      uint8_t edge,
                                      uint8_t hysteresis,
                                      uint16_t target_index);

/* FFT service */
void svc_freq_fft_bind_buffer(int16_t *buffer, uint16_t length);
void svc_freq_fft_prepare_from_u8(const uint8_t *buf,
                                  uint16_t length,
                                  uint32_t fs_hz,
                                  uint8_t window);
void svc_freq_fft_transform(void);

/* Signal generator service */
uint8_t svc_siggen_configure(uint8_t waveform,
                             uint32_t frequency_hz,
                             uint16_t vpp_mv,
                             int16_t offset_mv,
                             uint8_t duty_percent);

void svc_siggen_start(void);
void svc_siggen_stop(void);

Note
Only selected interfaces are shown.
The complete source code is not published to protect the original project work.