/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   config.h                                 */
/*    CPU:     ATxmega128A1                             */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   23.02.26                                 */
/*    Version: 1.6                                     */
/********************************************************/

#include <avr/io.h>
#pragma once

#ifndef F_CPU
#define F_CPU 32000000UL
#endif

#define UART_BAUD        115200UL

// Feature toggles (für Tests)
//#define XMP_ENABLE_LED        1

#define XMP_ENABLE_USART      1   // Für Phase 1A,auf 0 setzen
#define XMP_FRONTEND_RANGE_CODE   1U // 1 = x1 range (-5 V ... +5 V) und 2 = x0.1 range (-12 V ... +12 V)


// ====================== LED Konfiguration ======================
// Standard-Beispiel: LED an PD2, active-low (Pin zieht gegen GND)
/*#define LED_PORT         PORTA
#define LED_PIN_bm       PIN6_bm
#define LED_ACTIVE_LOW   1 */

/********************************************************/
/* Scope control inputs                                  */
/********************************************************/

/* Trigger level potentiometer on PA1 */
#define XMP_TRIG_POT_PORT        PORTA
#define XMP_TRIG_POT_bm          PIN1_bm

/* Volts/div button on PQ2 */
#define XMP_VDIV_BTN_PORT        PORTQ
#define XMP_VDIV_BTN_bm          PIN2_bm

/* Time/div button on PA6 */
#define XMP_TDIV_BTN_PORT        PORTA
#define XMP_TDIV_BTN_bm          PIN6_bm

/* Buttons are connected active-low */
#define XMP_BUTTON_ACTIVE_LOW    1

// ====================== USART Konfiguration =====================
// Standard: USARTE0 mit TX=PE3, RX=PE2 
#define XMP_USART        USARTE0
#define XMP_USART_PORT   PORTE
#define XMP_USART_TX_bm  PIN3_bm
#define XMP_USART_RX_bm  PIN2_bm

// ====================== I2C / TWI Konfiguration =================
// Hardware-TWI auf Port E (SDA = PE0, SCL = PE1) für den DS1803
#define XMP_I2C_PORT     PORTE
#define XMP_I2C_SDA_bm   PIN0_bm
#define XMP_I2C_SCL_bm   PIN1_bm

// ====================== System ======================
#define UI_TICK_HZ       1000U     // 1ms Tick

// =====  ADC =====

// Referenz (für Bring-up ok):
// - ADC_REFSEL_VCC_gc   = INTVCC = VCC/1.6
// - ADC_REFSEL_INT1V_gc = INT1V ~1.0V (kleiner, stabiler gg. VCC-Ripple)
#define XMP_ADC_REFSEL                 ADC_REFSEL_VCC_gc

// ADC Clock: 32MHz/16 = 2MHz 
#define XMP_ADC_PRESCALER              ADC_PRESCALER_DIV16_gc

// Vref in mV (Beispiel: bei VCC=3300mV und INTVCC=VCC/1.6 => ~2063mV)
#define XMP_ADC_VREF_MV                2063

// Offset nach Datenblatt
#define XMP_ADC_OFFSET_COUNTS              200

// Kalibrier-/Offset-Settings
#define XMP_ADC_USE_FACTORY_CALIB      1

// Digital Input Buffer aus (empfohlen)
#define XMP_ADC_DISABLE_DIGITAL_INPUT  1

//#define XMP_ADC_OFFSET_SAMPLES         256     // Mittelwert über 256
#define XMP_ADC_STATS_SAMPLES          13      // min/max/avg Fenster

// =====  Sampling =====
#define XMP_FS_HZ                 2000000U     // MHZ
#define XMP_ACQ_N                 4096U       // Samples pro Kanal

/********************************************************/
/* Function generator DAC configuration                 */
/********************************************************/

/*
 * DAC0 output pin:
 * DACA0 is available on PA2.
 */
#define XMP_GEN_DAC_PORT             PORTA
#define XMP_GEN_DAC_PIN_bm           PIN2_bm

/*
 * DAC reference selection test.
 *
 * Goal:
 * The function generator calculations are based on
 * Uref,DAC = 2.0625 V.
 *
 * This test checks whether this reference setting gives
 * the expected DAC output range.
 */
#define XMP_GEN_DAC_REFSEL           DAC_REFSEL_AVCC_gc

/*
 *  DAC reference voltage in millivolts.
 * 
 */
#define XMP_GEN_DAC_VREF_MV          3300u

/*
 * DAC resolution.
 */
#define XMP_GEN_DAC_MAX_CODE         4095u
#define XMP_GEN_DAC_MID_CODE         2048u
#define XMP_GEN_DAC_MIN_CODE         0u

/* ========================================== */
/* SSD1289 TFT DISPLAY MAPPING                */
/* ========================================== */

/* 16-bit parallel data bus */
#define XMP_LCD_DATA_LO_PORT     PORTC   /* D0..D7  */
#define XMP_LCD_DATA_HI_PORT     PORTF   /* D8..D15 */

/* Control signals */
#define XMP_LCD_CS_PORT          PORTB
#define XMP_LCD_CS_bm            PIN0_bm

#define XMP_LCD_RS_PORT          PORTB
#define XMP_LCD_RS_bm            PIN1_bm

#define XMP_LCD_WR_PORT          PORTB
#define XMP_LCD_WR_bm            PIN2_bm

#define XMP_LCD_RD_PORT          PORTB
#define XMP_LCD_RD_bm            PIN3_bm

/* Hardware Reset */
#define XMP_LCD_RST_PORT         PORTA
#define XMP_LCD_RST_bm           PIN4_bm

/* Backlight Control (BLCNT) */
#define XMP_LCD_BLCNT_PORT       PORTD
#define XMP_LCD_BLCNT_bm         PIN1_bm

/*
 * Backlight is active-low:
 * PD1 = 0 -> ON
 * PD1 = 1 -> OFF
 */
#define XMP_LCD_BLCNT_ACTIVE_LOW 1

/* ========================================== */
/* TOUCH PANEL CONTROLLER (SPI)               */
/* ========================================== */

/* TPIRO: Touch Panel Interrupt Request Output (Active Low) */
#define XMP_TP_IRQ_PORT          PORTA
#define XMP_TP_IRQ_bm            PIN5_bm

/* TPSCK: Touch Panel Serial Clock (SPI SCK) */
#define XMP_TP_SCK_PORT          PORTE
#define XMP_TP_SCK_bm            PIN7_bm

/* TPSO: Touch Panel Serial Out (SPI MISO - Master In Slave Out) */
#define XMP_TP_MISO_PORT         PORTE
#define XMP_TP_MISO_bm           PIN6_bm

/* TPSI: Touch Panel Serial In (SPI MOSI - Master Out Slave In) */
#define XMP_TP_MOSI_PORT         PORTE
#define XMP_TP_MOSI_bm           PIN5_bm

/* TPCS: Touch Panel Chip Select (Active Low) */
#define XMP_TP_CS_PORT           PORTE
#define XMP_TP_CS_bm             PIN4_bm

/* ========================================== */
/* TOUCH CALIBRATION                          */
/* ========================================== */

/*
 * First calibration values.
 * These values will be adjusted after testing.
 */
#define XMP_TOUCH_RAW_X_MIN      200u
#define XMP_TOUCH_RAW_X_MAX      3900u

#define XMP_TOUCH_RAW_Y_MIN      200u
#define XMP_TOUCH_RAW_Y_MAX      3900u

/*
 * Orientation correction.
 * Change these after first screen-coordinate test if needed.
 */
#define XMP_TOUCH_SWAP_XY        1u
#define XMP_TOUCH_INVERT_X       1u
#define XMP_TOUCH_INVERT_Y       0u

/* ========================================== */
/* TOUCH FILTER                               */
/* ========================================== */

#define XMP_TOUCH_FILTER_SAMPLES       16u
#define XMP_TOUCH_FILTER_MIN_VALID     8u

/* ========================================== */
/* BASIC COLORS (RGB565 Format)               */
/* ========================================== */
#define LCD_COLOR_BLACK   0x0000
#define LCD_COLOR_WHITE   0xFFFF
#define LCD_COLOR_RED     0xF800
#define LCD_COLOR_GREEN   0x07E0
#define LCD_COLOR_BLUE    0x001F
#define LCD_COLOR_GRAY    0x8410
#define LCD_COLOR_YELLOW  0xFFE0
#define LCD_COLOR_CYAN    0x07FFu
#define LCD_COLOR_DARK_CYAN   0x0451u
#define LCD_COLOR_DARK_GREEN  0x0320u
#define LCD_COLOR_DARK_GRAY   0x2104u
#define LCD_COLOR_LIGHT_GREEN 0xAFE5u