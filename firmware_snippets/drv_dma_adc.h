/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_dma_adc.h                            */
/*    CPU:     ATxmega128A1                             */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   31.03.26                                 */
/*    Version: 1.0                                      */
/********************************************************/

#pragma once

#include <stdint.h>

extern volatile uint8_t g_dma_done;

void drv_dma_adc0_init(uint8_t *dst, uint16_t n_bytes);
void drv_dma_adc0_start(void);
void drv_dma_adc0_stop(void);

/*
 * Re-arm DMA channel 0 for a new ADC frame.
 * This must be called before each new acquisition frame.
 */
void drv_dma_adc0_rearm(uint8_t *dst, uint16_t n_bytes);