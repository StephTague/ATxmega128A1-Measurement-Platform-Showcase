/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_clock.c                              */
/*    CPU:     ATxmega128A1-AU                          */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   19.02.26                                 */
/*    Version: 1.0                                      */
/********************************************************/
#include <avr/io.h>
#include "drv_clock.h"

void drv_clock_init_32mhz(void)
{
    // Internal 32 MHz RC oscillator enable
    OSC.CTRL |= OSC_RC32MEN_bm;
    while (!(OSC.STATUS & OSC_RC32MRDY_bm)) { ; }

    // Switch system clock to 32 MHz RC
    CCP = CCP_IOREG_gc;
    CLK.CTRL = CLK_SCLKSEL_RC32M_gc;

    // Prescalers = 1 (no division)
    CCP = CCP_IOREG_gc;
    CLK.PSCTRL = 0;
}