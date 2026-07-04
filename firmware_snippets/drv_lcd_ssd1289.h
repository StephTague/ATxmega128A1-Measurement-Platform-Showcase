/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_lcd_ssd1289.h                        */
/*    CPU:     ATxmega128A1                             */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   29.04.26                                 */
/*    Version: 1.2                                      */
/********************************************************/

#pragma once

#include <stdint.h>

void drv_ssd1289_init(void);

void drv_ssd1289_set_cursor(uint16_t x, uint16_t y);

void drv_ssd1289_draw_pixel_phys(uint16_t x,
                                 uint16_t y,
                                 uint16_t color);

void drv_ssd1289_fill_screen(uint16_t color);

/*
 * Draw 10 x 8 oscilloscope raster.
 */
void drv_ssd1289_draw_grid_fast(uint16_t bg_color,
                                uint16_t grid_color);

/*
 * Draw a 3-pixel thick trigger line.
 */
void drv_ssd1289_draw_trigger_line(uint16_t y,
                                   uint16_t color);

/*
 * Restore one horizontal row according to the 10 x 8 raster.
 */
void drv_ssd1289_restore_grid_row(uint16_t y,
                                  uint16_t bg_color,
                                  uint16_t grid_color);

/*
 * Restore the previous 3-pixel trigger line area.
 */
void drv_ssd1289_restore_trigger_area(uint16_t y,
                                      uint16_t bg_color,
                                      uint16_t grid_color);

/*
 * Update trigger line:
 * restore old position, draw new position.
 *
 * last_y must point to the previous trigger-line position.
 */
void drv_ssd1289_update_trigger_line(uint16_t new_y,
                                     uint16_t *last_y,
                                     uint16_t bg_color,
                                     uint16_t grid_color,
                                     uint16_t trigger_color);

/*
 * Draw a small 5x7 text string on the LCD.
 */
void drv_ssd1289_draw_text_small(uint16_t x,
                                 uint16_t y,
                                 const char *text,
                                 uint16_t fg_color,
                                 uint16_t bg_color);

/*
 * Clear a physical rectangle.
 */
void drv_ssd1289_fill_rect_phys(uint16_t x,
                                uint16_t y,
                                uint16_t w,
                                uint16_t h,
                                uint16_t color);

/*
 * Draw current V/div label at bottom-right.
 */
void drv_ssd1289_draw_vdiv_label(uint16_t mv_per_div);

 /* Draw current V/div label at bottom-right.
 */
void drv_ssd1289_draw_tdiv_label(uint16_t time_div_us);

