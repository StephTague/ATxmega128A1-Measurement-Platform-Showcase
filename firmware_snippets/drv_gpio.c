/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_gpio.c                               */
/*    CPU:     ATxmega128A1-AU                          */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   19.02.26                                 */
/*    Version: 1.0                                      */
/********************************************************/
#include <avr/io.h>
#include "drv_gpio.h"
#include "config.h"

void drv_gpio_init(void)
{
#if XMP_ENABLE_LED
    LED_PORT.DIRSET = LED_PIN_bm;

#if LED_ACTIVE_LOW
    LED_PORT.OUTSET = LED_PIN_bm;   // LED ein
#else
    LED_PORT.OUTCLR = LED_PIN_bm;   // LED aus
#endif
#endif
// --- 2. I2C (TWI) Konfiguration ---
    /* SDA und SCL Pins als Ausgang konfigurieren und auf HIGH (3.3V) setzen,
    um den I2C-Bus im Ruhezustand (Open-Drain) stabil zu halten.
	Hier nicht nötig, aber auskommentieren, wenn SDA und SCL nicht versorgt sind */
	
    /*XMP_I2C_PORT.DIRSET = XMP_I2C_SDA_bm | XMP_I2C_SCL_bm;
    XMP_I2C_PORT.OUTSET = XMP_I2C_SDA_bm | XMP_I2C_SCL_bm; */
}

void drv_led_on(void)
{
#if XMP_ENABLE_LED
#if LED_ACTIVE_LOW
    LED_PORT.OUTCLR = LED_PIN_bm;
#else
    LED_PORT.OUTSET = LED_PIN_bm;
#endif
#endif
}

void drv_led_off(void)
{
#if XMP_ENABLE_LED
#if LED_ACTIVE_LOW
    LED_PORT.OUTSET = LED_PIN_bm;
#else
    LED_PORT.OUTCLR = LED_PIN_bm;
#endif
#endif
}

void drv_led_toggle(void)
{
#if XMP_ENABLE_LED
    LED_PORT.OUTTGL = LED_PIN_bm;
#endif
}