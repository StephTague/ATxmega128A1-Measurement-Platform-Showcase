/*********************************************/
/* Projekt: XMEGA-Messplattform (XMP)        */
/* Datei:   drv_ds1803.h                     */
/* CPU:     ATxmega128A1                      */
/* Autor:   Lauris Taguetieu                 */
/* Datum:   30.03.26                         */
/* Version: 1.0                              */
/*********************************************/
#pragma once
#include <stdint.h>

// I2C-Adresse für den DS1803 (7-bit Format)
// Laut Schaltplan: 0 1 0 1 A2 A1 A0. 
// Da A2=A1=A0=0 -> 0101000 in Binär = 0x28
#define DS1803_I2C_ADDR         0x28

// Befehle laut DS1803 Datenblatt
#define DS1803_CMD_WRITE_POT0   0xA9    // Schreibt nur in Potentiometer 0
#define DS1803_CMD_WRITE_POT1   0xAA    // Schreibt nur in Potentiometer 1
#define DS1803_CMD_WRITE_BOTH   0xAF    // Schreibt denselben Wert in beide

// Initialisiert das TWI/I2C Modul des XMEGA an Port E
void drv_ds1803_init(void);

// Setzt den Wischerwert (0 bis 255) für die jeweiligen Zweige
void drv_ds1803_set_pot0(uint8_t wischer_wert);
void drv_ds1803_set_pot1(uint8_t wischer_wert);

// Hilfsfunktion: Setzt beide Hardware-Offsets auf einmal
void svc_hardware_offset_apply(uint8_t wert_oberer_zweig, uint8_t wert_ypos);
