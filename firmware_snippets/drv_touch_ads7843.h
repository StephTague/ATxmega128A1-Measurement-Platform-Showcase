/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_touch_ads7843.h                      */
/*    Zweck:   ADS7843/XPT2046 touch driver interface   */
/*    Datum:   28.05.2026                               */
/********************************************************/

#pragma once

#include <stdint.h>

/********************************************************/
/* Debug variables for Watch                            */
/********************************************************/

extern volatile uint8_t  g_touch_pressed;

extern volatile uint16_t g_touch_raw_x;
extern volatile uint16_t g_touch_raw_y;

extern volatile uint16_t g_touch_filt_raw_x;
extern volatile uint16_t g_touch_filt_raw_y;

extern volatile uint16_t g_touch_map_raw_x;
extern volatile uint16_t g_touch_map_raw_y;

extern volatile uint16_t g_touch_x;
extern volatile uint16_t g_touch_y;

extern volatile uint8_t  g_touch_valid_samples;

/********************************************************/
/* Public functions                                     */
/********************************************************/

void drv_touch_ads7843_init(void);

uint8_t drv_touch_ads7843_is_pressed(void);

uint16_t drv_touch_ads7843_read_raw_x(void);
uint16_t drv_touch_ads7843_read_raw_y(void);

uint8_t drv_touch_ads7843_read_raw(uint16_t *raw_x,
                                   uint16_t *raw_y);

uint8_t drv_touch_ads7843_read_raw_filtered(uint16_t *raw_x,
                                            uint16_t *raw_y);

uint8_t drv_touch_ads7843_read_screen(uint16_t *x,
                                      uint16_t *y);
uint8_t drv_touch_ads7843_is_pressed(void);