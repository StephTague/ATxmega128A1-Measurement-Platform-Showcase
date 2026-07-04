/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_usart.h                              */
/*    CPU:     ATxmega128A1-AU                          */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   19.02.26                                 */
/*    Version: 1.0                                      */
/********************************************************/
#pragma once
#include <stdint.h>

void drv_usart_init_115200(void);
void drv_usart_putc(char c);
void drv_usart_write(const char* s, uint16_t n);