/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_dma_adc.c                            */
/*    CPU:     ATxmega128A1                         */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   29.03.26                                 */
/*    Version: 1.0                                      */
/********************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "drv_dma_adc.h"

volatile uint8_t g_dma_done = 0;

/*
 * DMA trigger source for ADCA CH0.
 * Some AVR header packs use different macro names.
 * For ATxmega128A1, ADCA CH0 trigger is normally 0x10.
 */
#if defined(DMA_CH_TRIGSRC_ADCA_CH0_gc)
  #define XMP_DMA_TRIGSRC_ADCA_CH0   DMA_CH_TRIGSRC_ADCA_CH0_gc
#elif defined(DMA_CH_TRIGSRC_ADCA_CH0)
  #define XMP_DMA_TRIGSRC_ADCA_CH0   DMA_CH_TRIGSRC_ADCA_CH0
#else
  #define XMP_DMA_TRIGSRC_ADCA_CH0   0x10u
#endif

/*
 * XMEGA uses 24-bit DMA address registers.
 * In this project, SRAM and peripheral addresses are inside the lower
 * 64 kB address space, therefore the third address byte is set to 0.
 */
static inline void dma_set_addr16(volatile uint8_t *a0,
                                  volatile uint8_t *a1,
                                  volatile uint8_t *a2,
                                  uint16_t addr)
{
    *a0 = (uint8_t)(addr & 0xFFu);
    *a1 = (uint8_t)((addr >> 8) & 0xFFu);
    *a2 = 0u;
}

/*
 * Common configuration for DMA CH0:
 * Source      : ADCA.CH0.RESL
 * Destination : RAM buffer
 * Mode        : 1 byte per ADC conversion
 */
static void dma_adc0_configure(uint8_t *dst, uint16_t n_bytes)
{
    uint16_t src;
    uint16_t dst_addr;

    src = (uint16_t)&ADCA.CH0.RESL;
    dst_addr = (uint16_t)dst;

    dma_set_addr16(&DMA.CH0.SRCADDR0,
                   &DMA.CH0.SRCADDR1,
                   &DMA.CH0.SRCADDR2,
                   src);

    dma_set_addr16(&DMA.CH0.DESTADDR0,
                   &DMA.CH0.DESTADDR1,
                   &DMA.CH0.DESTADDR2,
                   dst_addr);

    DMA.CH0.TRFCNT = n_bytes;

    DMA.CH0.TRIGSRC = XMP_DMA_TRIGSRC_ADCA_CH0;

    /*
     * Source fixed: ADC result register.
     * Destination incrementing: RAM buffer.
     */
    DMA.CH0.ADDRCTRL =
        DMA_CH_SRCRELOAD_NONE_gc  | DMA_CH_SRCDIR_FIXED_gc |
        DMA_CH_DESTRELOAD_NONE_gc | DMA_CH_DESTDIR_INC_gc;

    /*
     * Enable transfer complete interrupt.
     */
    DMA.CH0.CTRLB = DMA_CH_TRNINTLVL_LO_gc;

    /*
     * 1 byte burst, single-shot per trigger.
     * Channel is not enabled here. It is enabled by drv_dma_adc0_start().
     */
    DMA.CH0.CTRLA = DMA_CH_BURSTLEN_1BYTE_gc | DMA_CH_SINGLE_bm;
}

void drv_dma_adc0_init(uint8_t *dst, uint16_t n_bytes)
{
    /*
     * Enable DMA controller.
     */
    DMA.CTRL = DMA_ENABLE_bm;

    /*
     * Clear software flag.
     */
    g_dma_done = 0;

    /*
     * Configure DMA channel 0 for the first frame.
     */
    dma_adc0_configure(dst, n_bytes);

    /*
     * Enable low level interrupts for DMA ISR.
     */
    PMIC.CTRL |= PMIC_LOLVLEN_bm;
    sei();
}

void drv_dma_adc0_rearm(uint8_t *dst, uint16_t n_bytes)
{
    /*
     * Disable DMA channel before changing transfer settings.
     */
    DMA.CH0.CTRLA &= ~DMA_CH_ENABLE_bm;

    /*
     * Clear software done flag.
     */
    g_dma_done = 0;

    /*
     * Clear DMA interrupt flags.
     * On XMEGA, interrupt flags are cleared by writing one.
     */
    DMA.CH0.CTRLB |= DMA_CH_TRNIF_bm;

#if defined(DMA_CH_ERRIF_bm)
    DMA.CH0.CTRLB |= DMA_CH_ERRIF_bm;
#endif

    /*
     * Make sure DMA controller remains enabled.
     */
    DMA.CTRL |= DMA_ENABLE_bm;

    /*
     * Reload addresses, transfer count and DMA mode.
     */
    dma_adc0_configure(dst, n_bytes);
}

void drv_dma_adc0_start(void)
{
    /*
     * Enable DMA channel.
     * The channel now waits for ADCA CH0 conversion results.
     */
    DMA.CH0.CTRLA |= DMA_CH_ENABLE_bm;
}

void drv_dma_adc0_stop(void)
{
    /*
     * Disable DMA channel.
     */
    DMA.CH0.CTRLA &= ~DMA_CH_ENABLE_bm;
}

ISR(DMA_CH0_vect)
{
    /*
     * Clear transfer complete flag.
     */
    DMA.CH0.CTRLB |= DMA_CH_TRNIF_bm;

    /*
     * Signal to the acquisition service that one frame is complete.
     */
    g_dma_done = 1u;
}