/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_adc.c                                */
/*    CPU:     ATxmega128A1-AU                          */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   23.02.26                                 */
/*    Version: 1.2                                    */
/********************************************************/
#include <avr/io.h>
#include <stddef.h>
#include "config.h"
#include "drv_adc.h"

/* -------- Calibration Row Read (XMEGA) -------- */
static uint8_t nvm_read_calib_byte(uint16_t index)
{
    uint8_t result;
    NVM.CMD = NVM_CMD_READ_CALIB_ROW_gc;
    __asm__ ("lpm %0, Z" : "=r"(result) : "z"(index));
    NVM.CMD = NVM_CMD_NO_OPERATION_gc;
    return result;
}

static void adca_load_factory_calibration(void)
{
#if XMP_ADC_USE_FACTORY_CALIB
    ADCA.CALL = nvm_read_calib_byte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0));
    ADCA.CALH = nvm_read_calib_byte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1));
#endif
}

static inline void adc_disable_digital_input_on_pins(void)
{
#if XMP_ADC_DISABLE_DIGITAL_INPUT
    // PA0/PA1 digital input buffer OFF (empfohlen)
    PORTA.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTA.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
#endif
}

void drv_adc_init(void)
{
#ifdef PR_ADC_bm
    PR.PRPA &= ~PR_ADC_bm;               // ADC Power Reduction aus (falls gesetzt)
#endif

    adc_disable_digital_input_on_pins();

    ADCA.CTRLA = 0;                      // ADC aus für Konfig
    ADCA.CTRLB = ADC_RESOLUTION_8BIT_gc; // unsigned, 8-bit
    ADCA.REFCTRL   = XMP_ADC_REFSEL;
    ADCA.PRESCALER = XMP_ADC_PRESCALER;

    // CH0 -> AIN0 (PA0)
    ADCA.CH0.CTRL    = ADC_CH_INPUTMODE_SINGLEENDED_gc;
    ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc;

    // CH1 -> AIN1 (PA1)
    ADCA.CH1.CTRL    = ADC_CH_INPUTMODE_SINGLEENDED_gc;
    ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc;

    // Phase 2: keine Channel-Interrupts
    ADCA.CH0.INTCTRL = ADC_CH_INTLVL_OFF_gc;
    ADCA.CH1.INTCTRL = ADC_CH_INTLVL_OFF_gc;
	
	// Event control: EVSEL=0 (Event Channel 0), EVACT=CH0
    // EVCTRL: [5:3] EVSEL, [2:0] EVACT
    // EVSEL=0 => 0<<3, EVACT=1 => 1
    ADCA.EVCTRL = (0u << 3) | (1u << 0); // or ADCA.EVCTRL = ADC_EVSEL_0123_gc | ADC_EVACT_CH0_gc;

    // Factory calibration vor ADC enable laden
    adca_load_factory_calibration();
	
	 
    ADCA.CTRLA = ADC_ENABLE_bm;          // ADC enable

    // Flags löschen
    ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
    ADCA.CH1.INTFLAGS = ADC_CH_CHIF_bm;
}

static inline uint8_t read_ch0(void)
{
    ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
    ADCA.CTRLA |= ADC_CH0START_bm;
    while(!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm)) {;}
    return ADCA.CH0.RES;
}

static inline uint8_t read_ch1(void)
{
    ADCA.CH1.INTFLAGS = ADC_CH_CHIF_bm;
    ADCA.CTRLA |= ADC_CH1START_bm;
    while(!(ADCA.CH1.INTFLAGS & ADC_CH_CHIF_bm)) {;}
    return ADCA.CH1.RES;
}

uint8_t drv_adc_read_raw(xmp_adc_ch_t ch)
{
    return (ch == XMP_ADC_CH0) ? read_ch0() : read_ch1();
}