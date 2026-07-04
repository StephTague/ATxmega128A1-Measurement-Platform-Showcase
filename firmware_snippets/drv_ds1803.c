/*****************************************************/
/* Projekt: XMEGA-Messplattform (XMP)                */
/* Datei:   drv_ds1803.c                             */
/* CPU:     ATxmega128A1                             */
/* Autor:   Lauris Taguetieu                         */
/* Datum:   30.03.26                                 */
/* Version: 1.0                                      */
/*****************************************************/
#include <avr/io.h>
#include "drv_ds1803.h"
#include "config.h"

// Baudraten-Berechnung für TWI (I2C) - 100 kHz Standard Mode
#define F_TWI 100000UL
#define TWI_BAUD_REG (uint8_t)(((F_CPU) / (2 * F_TWI)) - 5)

void drv_ds1803_init(void)
{
    // HINWEIS: Pin-Richtungen (DIRSET/OUTSET) werden zentral in drv_gpio_init() gesetzt!
    
    // TWI Master Modul auf Port E (TWIE) initialisieren
    TWIE.MASTER.BAUD = TWI_BAUD_REG;
    TWIE.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;         // TWI Master aktivieren
    TWIE.MASTER.CTRLB = 0;                            // Kein Timeout
    TWIE.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc; // Bus auf IDLE (frei) setzen
}

// Interne Hilfsfunktion für die I2C-Übertragung via TWIE
static void twi_write_ds1803(uint8_t cmd, uint8_t data)
{
    // Startbedingung und Slave-Adresse senden (Schreibmodus)
    TWIE.MASTER.ADDR = (DS1803_I2C_ADDR << 1) | 0;
    while(!(TWIE.MASTER.STATUS & TWI_MASTER_WIF_bm)); 

    // Befehlsbyte senden (Welches Poti?)
    TWIE.MASTER.DATA = cmd;
    while(!(TWIE.MASTER.STATUS & TWI_MASTER_WIF_bm));

    // Datenbyte senden (Wischerposition 0-255)
    TWIE.MASTER.DATA = data;
    while(!(TWIE.MASTER.STATUS & TWI_MASTER_WIF_bm));

    // Stoppbedingung senden
    TWIE.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
}

void drv_ds1803_set_pot0(uint8_t wischer_wert)
{
    twi_write_ds1803(DS1803_CMD_WRITE_POT0, wischer_wert);
}

void drv_ds1803_set_pot1(uint8_t wischer_wert)
{
    twi_write_ds1803(DS1803_CMD_WRITE_POT1, wischer_wert);
}

void svc_hardware_offset_apply(uint8_t wert_oberer_zweig, uint8_t wert_ypos)
{
    drv_ds1803_set_pot0(wert_ypos);
    drv_ds1803_set_pot1(wert_oberer_zweig );
}