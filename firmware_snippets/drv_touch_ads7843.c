/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_touch_ads7843.c                      */
/*    Zweck:   ADS7843/XPT2046 touch driver             */
/*    Datum:   28.05.2026                               */
/********************************************************/

#include <avr/io.h>
#include <stdint.h>

#include "config.h"
#include "drv_touch_ads7843.h"

/********************************************************/
/* ADS7843 commands                                     */
/********************************************************/

/*
 * Standard ADS7843 / XPT2046 12-bit commands.
 * X/Y can be swapped later by configuration.
 */
#define ADS7843_CMD_X     0xD0u
#define ADS7843_CMD_Y     0x90u

/********************************************************/
/* Debug variables for Watch                            */
/********************************************************/

volatile uint8_t  g_touch_pressed = 0u;

volatile uint16_t g_touch_raw_x = 0u;
volatile uint16_t g_touch_raw_y = 0u;

volatile uint16_t g_touch_filt_raw_x = 0u;
volatile uint16_t g_touch_filt_raw_y = 0u;

volatile uint16_t g_touch_map_raw_x = 0u;
volatile uint16_t g_touch_map_raw_y = 0u;

volatile uint16_t g_touch_x = 0u;
volatile uint16_t g_touch_y = 0u;

volatile uint8_t  g_touch_valid_samples = 0u;

/********************************************************/
/* Touch coordinate mapping                             */
/********************************************************/

static uint16_t touch_map_u16(uint16_t raw,
                              uint16_t raw_min,
                              uint16_t raw_max,
                              uint16_t screen_max)
{
    uint32_t value;

    if (raw_max <= raw_min)
    {
        return 0u;
    }

    if (raw < raw_min)
    {
        raw = raw_min;
    }

    if (raw > raw_max)
    {
        raw = raw_max;
    }

    value = (uint32_t)(raw - raw_min);
    value = value * (uint32_t)screen_max;
    value = value / (uint32_t)(raw_max - raw_min);

    return (uint16_t)value;
}

/********************************************************/
/* Low-level pin helpers                                */
/********************************************************/

static void tp_cs_low(void)
{
    XMP_TP_CS_PORT.OUTCLR = XMP_TP_CS_bm;
}

static void tp_cs_high(void)
{
    XMP_TP_CS_PORT.OUTSET = XMP_TP_CS_bm;
}

static void tp_sck_low(void)
{
    XMP_TP_SCK_PORT.OUTCLR = XMP_TP_SCK_bm;
}

static void tp_sck_high(void)
{
    XMP_TP_SCK_PORT.OUTSET = XMP_TP_SCK_bm;
}

static void tp_mosi_low(void)
{
    XMP_TP_MOSI_PORT.OUTCLR = XMP_TP_MOSI_bm;
}

static void tp_mosi_high(void)
{
    XMP_TP_MOSI_PORT.OUTSET = XMP_TP_MOSI_bm;
}

static uint8_t tp_miso_read(void)
{
    if ((XMP_TP_MISO_PORT.IN & XMP_TP_MISO_bm) != 0u)
    {
        return 1u;
    }

    return 0u;
}

/*
 * Small delay for bit-banged SPI.
 * Intentionally slow for stable ADS7843 validation.
 */
static void tp_delay(void)
{
    volatile uint8_t i;

    for (i = 0u; i < 8u; i++)
    {
        __asm__ __volatile__("nop");
    }
}

/********************************************************/
/* Bit-banged SPI                                       */
/********************************************************/

static uint8_t tp_spi_transfer(uint8_t data)
{
    uint8_t i;
    uint8_t rx = 0u;

    for (i = 0u; i < 8u; i++)
    {
        rx <<= 1u;

        if ((data & 0x80u) != 0u)
        {
            tp_mosi_high();
        }
        else
        {
            tp_mosi_low();
        }

        tp_delay();

        tp_sck_high();
        tp_delay();

        if (tp_miso_read() != 0u)
        {
            rx |= 1u;
        }

        tp_sck_low();
        tp_delay();

        data <<= 1u;
    }

    return rx;
}

/********************************************************/
/* ADS7843 read                                         */
/********************************************************/

static uint16_t ads7843_read_channel(uint8_t command)
{
    uint8_t hi;
    uint8_t lo;
    uint16_t value;

    tp_cs_low();
    tp_delay();

    /*
     * Send command.
     */
    (void)tp_spi_transfer(command);

    /*
     * Read 12-bit conversion result.
     * ADS7843 returns data left-aligned in the 16 received bits.
     */
    hi = tp_spi_transfer(0x00u);
    lo = tp_spi_transfer(0x00u);

    tp_cs_high();

    value = (uint16_t)((((uint16_t)hi << 8u) | (uint16_t)lo) >> 3u);
    value &= 0x0FFFu;

    return value;
}

/********************************************************/
/* Public functions                                     */
/********************************************************/

void drv_touch_ads7843_init(void)
{
    /*
     * Outputs:
     * CS, SCK, MOSI
     */
    XMP_TP_CS_PORT.DIRSET   = XMP_TP_CS_bm;
    XMP_TP_SCK_PORT.DIRSET  = XMP_TP_SCK_bm;
    XMP_TP_MOSI_PORT.DIRSET = XMP_TP_MOSI_bm;

    /*
     * Inputs:
     * MISO, IRQ
     */
    XMP_TP_MISO_PORT.DIRCLR = XMP_TP_MISO_bm;
    XMP_TP_IRQ_PORT.DIRCLR  = XMP_TP_IRQ_bm;

    /*
     * Idle states.
     */
    tp_cs_high();
    tp_sck_low();
    tp_mosi_low();

    /*
     * IRQ is PA5 in the current hardware mapping.
     * PENIRQ / TPIRO is active low.
     */
    PORTA.PIN5CTRL = PORT_OPC_PULLUP_gc;

    g_touch_pressed = 0u;
    g_touch_raw_x = 0u;
    g_touch_raw_y = 0u;
    g_touch_filt_raw_x = 0u;
    g_touch_filt_raw_y = 0u;
    g_touch_map_raw_x = 0u;
    g_touch_map_raw_y = 0u;
    g_touch_x = 0u;
    g_touch_y = 0u;
    g_touch_valid_samples = 0u;
}

uint8_t drv_touch_ads7843_is_pressed(void)
{
    /*
     * PENIRQ / TPIRO is active low.
     */
    if ((XMP_TP_IRQ_PORT.IN & XMP_TP_IRQ_bm) == 0u)
    {
        return 1u;
    }

    return 0u;
}

uint16_t drv_touch_ads7843_read_raw_x(void)
{
    return ads7843_read_channel(ADS7843_CMD_X);
}

uint16_t drv_touch_ads7843_read_raw_y(void)
{
    return ads7843_read_channel(ADS7843_CMD_Y);
}

uint8_t drv_touch_ads7843_read_raw(uint16_t *raw_x,
                                   uint16_t *raw_y)
{
    uint16_t x1;
    uint16_t y1;

    if ((raw_x == 0) || (raw_y == 0))
    {
        return 0u;
    }

    if (!drv_touch_ads7843_is_pressed())
    {
        g_touch_pressed = 0u;
        return 0u;
    }

    x1 = drv_touch_ads7843_read_raw_x();
    y1 = drv_touch_ads7843_read_raw_y();

    *raw_x = x1;
    *raw_y = y1;

    g_touch_pressed = 1u;
    g_touch_raw_x = x1;
    g_touch_raw_y = y1;

    return 1u;
}

uint8_t drv_touch_ads7843_read_raw_filtered(uint16_t *raw_x,
                                            uint16_t *raw_y)
{
    uint8_t i;
    uint8_t valid;

    uint16_t rx;
    uint16_t ry;

    uint32_t sum_x;
    uint32_t sum_y;

    if ((raw_x == 0) || (raw_y == 0))
    {
        return 0u;
    }

    if (!drv_touch_ads7843_is_pressed())
    {
        g_touch_pressed = 0u;
        g_touch_valid_samples = 0u;
        return 0u;
    }

    /*
     * Dummy read.
     * The first conversion after command/touch can be unstable.
     */
    (void)drv_touch_ads7843_read_raw_x();
    (void)drv_touch_ads7843_read_raw_y();

    sum_x = 0u;
    sum_y = 0u;
    valid = 0u;

    for (i = 0u; i < XMP_TOUCH_FILTER_SAMPLES; i++)
    {
        if (!drv_touch_ads7843_is_pressed())
        {
            break;
        }

        rx = drv_touch_ads7843_read_raw_x();
        ry = drv_touch_ads7843_read_raw_y();

        sum_x += (uint32_t)rx;
        sum_y += (uint32_t)ry;

        valid++;
    }

    if (valid < XMP_TOUCH_FILTER_MIN_VALID)
    {
        g_touch_pressed = 0u;
        g_touch_valid_samples = valid;
        return 0u;
    }

    rx = (uint16_t)(sum_x / (uint32_t)valid);
    ry = (uint16_t)(sum_y / (uint32_t)valid);

    *raw_x = rx;
    *raw_y = ry;

    g_touch_pressed = 1u;
    g_touch_valid_samples = valid;

    g_touch_filt_raw_x = rx;
    g_touch_filt_raw_y = ry;

    /*
     * For Watch/debug:
     * Store filtered values also as current raw values.
     */
    g_touch_raw_x = rx;
    g_touch_raw_y = ry;

    return 1u;
}

uint8_t drv_touch_ads7843_read_screen(uint16_t *x,
                                      uint16_t *y)
{
    uint16_t raw_x;
    uint16_t raw_y;

    uint16_t sx;
    uint16_t sy;

    uint16_t temp;

    if ((x == 0) || (y == 0))
    {
        return 0u;
    }

    /*
     * Use filtered raw values for stable screen coordinates.
     */
    if (!drv_touch_ads7843_read_raw_filtered(&raw_x, &raw_y))
    {
        return 0u;
    }

    /*
     * IMPORTANT:
     * Swap must be done before mapping.
     * X must always be mapped to 0...319.
     * Y must always be mapped to 0...239.
     */
#if XMP_TOUCH_SWAP_XY
    temp = raw_x;
    raw_x = raw_y;
    raw_y = temp;
#endif

    /*
     * Debug values after swap, before mapping.
     */
    g_touch_map_raw_x = raw_x;
    g_touch_map_raw_y = raw_y;

    sx = touch_map_u16(raw_x,
                       XMP_TOUCH_RAW_X_MIN,
                       XMP_TOUCH_RAW_X_MAX,
                       319u);

    sy = touch_map_u16(raw_y,
                       XMP_TOUCH_RAW_Y_MIN,
                       XMP_TOUCH_RAW_Y_MAX,
                       239u);

#if XMP_TOUCH_INVERT_X
    sx = (uint16_t)(319u - sx);
#endif

#if XMP_TOUCH_INVERT_Y
    sy = (uint16_t)(239u - sy);
#endif

    *x = sx;
    *y = sy;

    g_touch_x = sx;
    g_touch_y = sy;

    return 1u;
}