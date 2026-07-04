/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_gpio.h                               */
/*    CPU:     ATxmega128A1-AU                          */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   19.02.26                                 */
/*    Version: 1.0                                      */
/********************************************************/
#pragma once
#include <stdint.h>

void drv_gpio_init(void);

void drv_led_on(void);
void drv_led_off(void);
void drv_led_toggle(void);