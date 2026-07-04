/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_usart.c                              */
/*    CPU:     ATxmega128A1-AU                          */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   19.02.26                                 */
/*    Version: 1.0                                      */
/********************************************************/
#include <avr/io.h>
#include "drv_usart.h"
#include "config.h"

// Baud params für 32MHz, 115200: BSCALE=-4, BSEL=277 (sehr geringe Abweichung)
#define BSEL_115200   277
#define BSCALE_115200 (-4)

static inline uint8_t bscale_to_reg(int8_t bscale)
{
    // 4-bit signed in upper nibble (two's complement)
    return (uint8_t)((bscale < 0) ? (16 + bscale) : bscale);
}

void drv_usart_init_115200(void)
{
#if XMP_ENABLE_USART
    // TX as output, RX as input
    XMP_USART_PORT.DIRSET = XMP_USART_TX_bm;
    XMP_USART_PORT.DIRCLR = XMP_USART_RX_bm;

    XMP_USART.BAUDCTRLA = (uint8_t)(BSEL_115200 & 0xFF);
    XMP_USART.BAUDCTRLB = (uint8_t)((bscale_to_reg(BSCALE_115200) << 4) | (BSEL_115200 >> 8));

    XMP_USART.CTRLC = USART_CHSIZE_8BIT_gc;                  // 8N1
    XMP_USART.CTRLB = USART_TXEN_bm | USART_RXEN_bm;         // enable Tx/Rx
#endif
}

void drv_usart_putc(char c)
{
#if XMP_ENABLE_USART
    while(!(XMP_USART.STATUS & USART_DREIF_bm)) { ; }
    XMP_USART.DATA = (uint8_t)c;
#else
    (void)c;
#endif
}

void drv_usart_write(const char* s, uint16_t n)
{
#if XMP_ENABLE_USART
    for(uint16_t i = 0; i < n; i++)
        drv_usart_putc(s[i]);
#else
    (void)s; (void)n;
#endif
}