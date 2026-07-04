/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_adc.h                                */
/*    CPU:     ATxmega128A1-AU                          */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   23.02.26                                 */
/*    Version: 1.1                                      */
/********************************************************/
#pragma once
#include <stdint.h>

typedef enum {
    XMP_ADC_CH0 = 0,   // PA0 (AIN0)
    XMP_ADC_CH1 = 1    // PA1 (AIN1)
} xmp_adc_ch_t;

void     drv_adc_init(void);
uint8_t drv_adc_read_raw(xmp_adc_ch_t ch);
