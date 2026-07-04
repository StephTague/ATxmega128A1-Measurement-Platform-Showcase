/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   svc_spectrum_renderer.h                  */
/*    CPU:     ATxmega128A1                             */
/*    Zweck:   Spectrum renderer for FFT analyzer       */
/********************************************************/
#pragma once

#include <stdint.h>
#include "svc_freq_fft.h"

void svc_spectrum_renderer_reset(void);
void svc_spectrum_renderer_draw_basic(const freq_fft_exec_t *fft_result);
