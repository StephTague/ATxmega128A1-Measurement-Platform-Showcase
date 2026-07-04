/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   drv_lcd_ssd1289.c                        */
/*    CPU:     ATxmega128A1                             */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   29.04.26                                 */
/*    Version: 1.2                                      */
/********************************************************/

#include "drv_lcd_ssd1289.h"
#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

/* Helper function for physical pin initialization */
static void drv_ssd1289_gpio_init(void) {
    /* Set Data Ports (Low and High) as Output */
    XMP_LCD_DATA_LO_PORT.DIR = 0xFF;
    XMP_LCD_DATA_HI_PORT.DIR = 0xFF;

    /* Set Control Pins as Output without disturbing JTAG (PB4-PB7) */
    XMP_LCD_CS_PORT.DIRSET  = XMP_LCD_CS_bm;
    XMP_LCD_RS_PORT.DIRSET  = XMP_LCD_RS_bm;
    XMP_LCD_WR_PORT.DIRSET  = XMP_LCD_WR_bm;
    XMP_LCD_RD_PORT.DIRSET  = XMP_LCD_RD_bm;
    XMP_LCD_RST_PORT.DIRSET = XMP_LCD_RST_bm;

    /* Set initial states: Control pins HIGH (Idle state) */
    XMP_LCD_CS_PORT.OUTSET  = XMP_LCD_CS_bm;
    XMP_LCD_RS_PORT.OUTSET  = XMP_LCD_RS_bm;
    XMP_LCD_WR_PORT.OUTSET  = XMP_LCD_WR_bm;
    XMP_LCD_RD_PORT.OUTSET  = XMP_LCD_RD_bm;
    XMP_LCD_RST_PORT.OUTSET = XMP_LCD_RST_bm;
}

/* Write command to the LCD register */
static void LCD_Write_Cmd(uint16_t cmd) {
    XMP_LCD_RS_PORT.OUTCLR = XMP_LCD_RS_bm; // RS = 0 for Command
    XMP_LCD_CS_PORT.OUTCLR = XMP_LCD_CS_bm; // CS = 0 to Select

    XMP_LCD_DATA_HI_PORT.OUT = (uint8_t)(cmd >> 8);
    XMP_LCD_DATA_LO_PORT.OUT = (uint8_t)(cmd & 0xFF);

    XMP_LCD_WR_PORT.OUTCLR = XMP_LCD_WR_bm; // WR = 0 to Write
    XMP_LCD_WR_PORT.OUTSET = XMP_LCD_WR_bm; // WR = 1 to Latch
    XMP_LCD_CS_PORT.OUTSET = XMP_LCD_CS_bm; // CS = 1 to Deselect
}

/* Write data to the LCD GRAM or Register */
static void LCD_Write_Data(uint16_t data) {
    XMP_LCD_RS_PORT.OUTSET = XMP_LCD_RS_bm; // RS = 1 for Data
    XMP_LCD_CS_PORT.OUTCLR = XMP_LCD_CS_bm; // CS = 0 to Select

    XMP_LCD_DATA_HI_PORT.OUT = (uint8_t)(data >> 8);
    XMP_LCD_DATA_LO_PORT.OUT = (uint8_t)(data & 0xFF);

    XMP_LCD_WR_PORT.OUTCLR = XMP_LCD_WR_bm; // WR = 0 to Write
    XMP_LCD_WR_PORT.OUTSET = XMP_LCD_WR_bm; // WR = 1 to Latch
    XMP_LCD_CS_PORT.OUTSET = XMP_LCD_CS_bm; // CS = 1 to Deselect
}

/* Write value to a specific Register */
static inline void LCD_Write_Reg(uint16_t reg, uint16_t value) {
    LCD_Write_Cmd(reg);
    LCD_Write_Data(value);
}

void drv_ssd1289_init(void) {
    drv_ssd1289_gpio_init();

    /* --- HARDWARE RESET --- */
    XMP_LCD_RST_PORT.OUTCLR = XMP_LCD_RST_bm;
    _delay_ms(50);
    XMP_LCD_RST_PORT.OUTSET = XMP_LCD_RST_bm;
    _delay_ms(150);

    /* --- WAKE UP SEQUENCE (Datasheet Page 68) --- */
    LCD_Write_Reg(0x0007, 0x0021);
    LCD_Write_Reg(0x0000, 0x0001); /* Enable Oscillator */
    LCD_Write_Reg(0x0007, 0x0023);
    LCD_Write_Reg(0x0010, 0x0000); /* Exit Sleep Mode */
    _delay_ms(15);

    /* --- POWER CONFIGURATION --- */
    LCD_Write_Reg(0x0003, 0xA8A4);
    LCD_Write_Reg(0x000C, 0x0000);
    LCD_Write_Reg(0x000D, 0x080C);
    LCD_Write_Reg(0x000E, 0x2B00);
    LCD_Write_Reg(0x001E, 0x00B7);

    /* --- DISPLAY & RAM CONFIGURATION --- */
    LCD_Write_Reg(0x0001, 0x2B3F);
    LCD_Write_Reg(0x0002, 0x0600);

    /* Entry Mode: ID0=1, ID1=1 for standard X/Y incrementation */
    LCD_Write_Reg(0x0011, 0x6070);
    _delay_ms(15);

    /*  WINDOW RAM LIMITS (Datasheet Page 44) --- */
    /* Prevents the cursor from freezing out of bounds */
    LCD_Write_Reg(0x0044, 0xEF00); /* Horizontal RAM start(0) end(239) */
    LCD_Write_Reg(0x0045, 0x0000); /* Vertical RAM start(0) */
    LCD_Write_Reg(0x0046, 0x013F); /* Vertical RAM end(319) */

    /* --- CURSOR INITIALIZATION --- */
    LCD_Write_Reg(0x004E, 0x0000); /* X = 0 */
    LCD_Write_Reg(0x004F, 0x0000); /* Y = 0 */

    /* --- FINAL DISPLAY TURN ON --- */
    LCD_Write_Reg(0x0007, 0x0233);
}

void drv_ssd1289_set_cursor(uint16_t x, uint16_t y) {
    /* Validate boundaries */
    if(x >= 320 || y >= 240) return;

    /* Set X and Y address */
    LCD_Write_Reg(0x004E, y); /* Note: SSD1289 maps physical X to logical Y when rotated */
    LCD_Write_Reg(0x004F, x);

    /* Prepare for RAM write */
    LCD_Write_Cmd(0x0022);
}

void drv_ssd1289_draw_pixel_phys(uint16_t x, uint16_t y, uint16_t color) {
    drv_ssd1289_set_cursor(x, y);
    LCD_Write_Data(color);
}

void drv_ssd1289_fill_screen(uint16_t color) {
    uint32_t i;

    /* Reset cursor to top-left */
    drv_ssd1289_set_cursor(0, 0);

    /* High-speed burst write */
    XMP_LCD_RS_PORT.OUTSET = XMP_LCD_RS_bm; // RS = 1
    XMP_LCD_CS_PORT.OUTCLR = XMP_LCD_CS_bm; // CS = 0

    XMP_LCD_DATA_HI_PORT.OUT = (uint8_t)(color >> 8);
    XMP_LCD_DATA_LO_PORT.OUT = (uint8_t)(color & 0xFF);

    /* Fast toggle for 320x240 pixels (76800 iterations) */
    for(i = 0; i < 76800UL; i++) {
        XMP_LCD_WR_PORT.OUTCLR = XMP_LCD_WR_bm;
        XMP_LCD_WR_PORT.OUTSET = XMP_LCD_WR_bm;
    }
    XMP_LCD_CS_PORT.OUTSET = XMP_LCD_CS_bm;
}

/********************************************************/
/* Added helper functions for oscilloscope UI            */
/********************************************************/

/*
 * Return the correct color for one pixel of the 10 x 8 raster.
 *
 * Screen:
 * - width  = 320 px
 * - height = 240 px
 *
 * Raster:
 * - 10 horizontal divisions => vertical line every 32 px
 * - 8 vertical divisions    => horizontal line every 30 px
 */
static uint16_t drv_ssd1289_grid_pixel_color(uint16_t x,
                                             uint16_t y,
                                             uint16_t bg_color,
                                             uint16_t grid_color)
{
    if ((x % 32u) == 0u ||
        (y % 30u) == 0u ||
        x == 319u ||
        y == 239u)
    {
        return grid_color;
    }

    return bg_color;
}

/*
 * Draw the complete 10 x 8 raster.
 *
 * This uses the already validated pixel function.
 * It is not the fastest possible solution, but it keeps the
 * proven LCD communication path unchanged.
 */
void drv_ssd1289_draw_grid_fast(uint16_t bg_color,
                                uint16_t grid_color)
{
    uint16_t x;
    uint16_t y;
    uint16_t color;

    for (y = 0u; y < 240u; y++)
    {
        for (x = 0u; x < 320u; x++)
        {
            color = drv_ssd1289_grid_pixel_color(x, y, bg_color, grid_color);
            drv_ssd1289_draw_pixel_phys(x, y, color);
        }
    }
}

/*
 * Restore one horizontal row according to the raster.
 *
 * This is used to erase the old trigger line cleanly.
 */
void drv_ssd1289_restore_grid_row(uint16_t y,
                                  uint16_t bg_color,
                                  uint16_t grid_color)
{
    uint16_t x;
    uint16_t color;

    if (y >= 240u)
    {
        return;
    }

    for (x = 0u; x < 320u; x++)
    {
        color = drv_ssd1289_grid_pixel_color(x, y, bg_color, grid_color);
        drv_ssd1289_draw_pixel_phys(x, y, color);
    }
}

/*
 * Restore the 3-pixel high area used by the trigger line.
 */
void drv_ssd1289_restore_trigger_area(uint16_t y,
                                      uint16_t bg_color,
                                      uint16_t grid_color)
{
    drv_ssd1289_restore_grid_row(y, bg_color, grid_color);

    if (y > 0u)
    {
        drv_ssd1289_restore_grid_row(y - 1u, bg_color, grid_color);
    }

    if (y < 239u)
    {
        drv_ssd1289_restore_grid_row(y + 1u, bg_color, grid_color);
    }
}

/*
 * Draw a 3-pixel thick horizontal trigger line.
 *
 * This is the same logic that was already validated in main.c.
 */
void drv_ssd1289_draw_trigger_line(uint16_t y,
                                   uint16_t color)
{
    uint16_t x;

    if (y >= 240u)
    {
        return;
    }

    for (x = 0u; x < 320u; x++)
    {
        drv_ssd1289_draw_pixel_phys(x, y, color);

        if (y > 0u)
        {
            drv_ssd1289_draw_pixel_phys(x, y - 1u, color);
        }

        if (y < 239u)
        {
            drv_ssd1289_draw_pixel_phys(x, y + 1u, color);
        }
    }
}

/*
 * Update trigger line:
 * 1. restore old trigger area
 * 2. draw new trigger line
 * 3. store new position
 *
 * This keeps main.c clean.
 */
void drv_ssd1289_update_trigger_line(uint16_t new_y,
                                     uint16_t *last_y,
                                     uint16_t bg_color,
                                     uint16_t grid_color,
                                     uint16_t trigger_color)
{
    if (last_y == 0)
    {
        return;
    }

    if (new_y >= 240u)
    {
        new_y = 239u;
    }

    if (*last_y == new_y)
    {
        return;
    }

    if (*last_y < 240u)
    {
        drv_ssd1289_restore_trigger_area(*last_y, bg_color, grid_color);
    }

    drv_ssd1289_draw_trigger_line(new_y, trigger_color);

    *last_y = new_y;
}

/********************************************************/
/* Small text drawing for scope UI                      */
/********************************************************/

/*
 * Text orientation settings.
 *
 * Here:
 * - characters are drawn normally
 * - only the string order is reversed for the LCD orientation
 */
#define LCD_TEXT_REVERSE_STRING  1
#define LCD_TEXT_SCALE           1u

/*
 * Slightly thicker text.
 * 0 = normal 5x7
 * 1 = slightly bold, but not as large as scale 2
 */
#define LCD_TEXT_BOLD            1u

/*
 * Corrected digit font.
 *
 * The digit bitmaps are horizontally pre-mirrored because the LCD text
 * rendering already reverses the string order. This keeps numbers readable
 * without changing the working m and V letters.
 */
static const uint8_t font_digits_5x7[10][5] =
{
    {0x3E, 0x45, 0x49, 0x51, 0x3E}, /* 0 */
    {0x00, 0x40, 0x7F, 0x42, 0x00}, /* 1 */
    {0x46, 0x49, 0x51, 0x61, 0x42}, /* 2 */
    {0x31, 0x4B, 0x45, 0x41, 0x21}, /* 3 */
    {0x10, 0x7F, 0x12, 0x14, 0x18}, /* 4 */
    {0x39, 0x45, 0x45, 0x45, 0x27}, /* 5 */
    {0x30, 0x49, 0x49, 0x4A, 0x3C}, /* 6 */
    {0x03, 0x05, 0x09, 0x71, 0x01}, /* 7 */
    {0x36, 0x49, 0x49, 0x49, 0x36}, /* 8 */
    {0x1E, 0x29, 0x49, 0x49, 0x06}  /* 9 */
};

static const uint8_t font_space_5x7[5] =
{
    0x00, 0x00, 0x00, 0x00, 0x00
};

/*
 * Lowercase m.
 */
static const uint8_t font_m_lower_5x7[5] =
{
    0x7C, 0x04, 0x18, 0x04, 0x78
};

/*
 * Uppercase V.
 */
static const uint8_t font_v_upper_5x7[5] =
{
    0x1F, 0x20, 0x40, 0x20, 0x1F
};

/*
 * Lowercase u.
 * Horizontally pre-mirrored, like digits, so it appears correct
 * with LCD_TEXT_REVERSE_STRING = 1.
 */
static const uint8_t font_u_lower_5x7[5] =
{
    0x7C, 0x20, 0x40, 0x40, 0x3C
};

/*
 * Lowercase s.
 * Horizontally pre-mirrored, like digits, so it appears correct
 * with LCD_TEXT_REVERSE_STRING = 1.
 */
static const uint8_t font_s_lower_5x7[5] =
{
    0x24, 0x54, 0x54, 0x54, 0x48
};

static const uint8_t *drv_ssd1289_get_char_bitmap(char c)
{
	/* Character mapping for the LCD display */
	static const uint8_t f_dot[5]   = {0x00, 0x60, 0x60, 0x00, 0x00}; /* . */
	static const uint8_t f_col[5]   = {0x00, 0x36, 0x36, 0x00, 0x00}; /* : */
	static const uint8_t f_eq[5]    = {0x14, 0x14, 0x14, 0x14, 0x14}; /* = */
	static const uint8_t f_a[5]     = {0x78, 0x54, 0x54, 0x54, 0x20}; /* a */
	static const uint8_t f_f[5]     = {0x02, 0x01, 0x09, 0x7E, 0x08}; /* f */
	static const uint8_t f_i[5]     = {0x00, 0x40, 0x7D, 0x44, 0x00}; /* i */
	static const uint8_t f_K[5]     = {0x41, 0x22, 0x14, 0x08, 0x7F}; /* K */
	static const uint8_t f_H[5]     = {0x7F, 0x08, 0x08, 0x08, 0x7F}; /* H */
	static const uint8_t f_z[5]     = {0x43, 0x45, 0x49, 0x51, 0x61}; /* z */
	static const uint8_t f_n[5]     = {0x7F, 0x10, 0x08, 0x04, 0x7F}; /* n */
	static const uint8_t f_X[5] = {0x63, 0x14, 0x08, 0x14, 0x63}; /* x */
	static const uint8_t f_T[5]     = {0x01, 0x01, 0x7F, 0x01, 0x01}; /* T */
	static const uint8_t f_R[5]     = {0x46, 0x29, 0x19, 0x09, 0x7F}; /* R */
	static const uint8_t f_I[5]     = {0x00, 0x41, 0x7F, 0x41, 0x00}; /* I */
	static const uint8_t f_G[5]     = {0x7A, 0x49, 0x49, 0x41, 0x3E}; /* G */
	static const uint8_t f_W[5]     = {0x7F, 0x20, 0x18, 0x20, 0x7F}; /* W */
	static const uint8_t f_A[5]     = {0x7E, 0x11, 0x11, 0x11, 0x7E}; /* A */
	static const uint8_t f_D[5]     = {0x1C, 0x22, 0x41, 0x41, 0x7F}; /* D */
	static const uint8_t f_pe[5]     = {0x62, 0x64, 0x08, 0x13, 0x23}; /* % */
	static const uint8_t f_E[5]     = {0x41, 0x49, 0x49, 0x49, 0x7F}; /* E */
	static const uint8_t f_L[5]     = {0x40, 0x40, 0x40, 0x40, 0x7F}; /* L */
	static const uint8_t f_N[5]     = {0x7F, 0x10, 0x08, 0x04, 0x7F}; /* N */
	static const uint8_t f_P[5]     = {0x06, 0x09, 0x09, 0x09, 0x7F}; /* P */
	static const uint8_t f_Q[5]     = {0x5E, 0x21, 0x51, 0x41, 0x3E};	/* Q */
	static const uint8_t f_Y[5]     = {0x03, 0x04, 0x78, 0x04, 0x03};	/* Y */
	static const uint8_t f_F[5]     = {0x01, 0x09, 0x09, 0x09, 0x7F};	/* F */
    static const uint8_t f_copy[5]  = {0x3E, 0x55, 0x5D, 0x41, 0x3E}; /* © */
	static const uint8_t f_C[5]     = {0x22, 0x41, 0x41, 0x41, 0x3E}; /* C */
	static const uint8_t f_M[5]     = {0x7F, 0x02, 0x04, 0x02, 0x7F}; /* M */
	static const uint8_t f_O[5]     = {0x3E, 0x41, 0x41, 0x41, 0x3E}; /* O */
	static const uint8_t f_S[5]     = {0x31, 0x49, 0x49, 0x49, 0x46}; /* S */
	static const uint8_t f_c[5]     = {0x20, 0x44, 0x44, 0x44, 0x38}; /* c */
	static const uint8_t f_g[5]     = {0x3E, 0x52, 0x52, 0x52, 0x0C}; /* g */
	static const uint8_t f_h[5]     = {0x78, 0x04, 0x04, 0x08, 0x7F}; /* h  */
	static const uint8_t f_r[5]     = {0x00, 0x04, 0x04, 0x08, 0x7C}; /* r  */
	static const uint8_t f_t[5]     = {0x20, 0x40, 0x44, 0x3F, 0x04}; /* t */
	static const uint8_t f_y[5]     = {0x3C, 0x50, 0x50, 0x50, 0x4C}; /* y*/
    static const uint8_t f_o[5]     = {0x38, 0x44, 0x44, 0x44, 0x38}; /* o  */
    static const uint8_t f_p[5]     = {0x18, 0x24, 0x24, 0x24, 0xFC}; /* p */
	static const uint8_t f_minus[5] = {0x08, 0x08, 0x08, 0x08, 0x08}; /* - */
	static const uint8_t f_U[5]     = {0x3F, 0x40, 0x40, 0x40, 0x3F}; /* U */
	static const uint8_t f_u[5]     = {0x3C, 0x40, 0x40, 0x40, 0x7C}; /* u */	
	static const uint8_t f_plus[5]  = {0x08, 0x08, 0x3E, 0x08, 0x08}; /* + */	
	if (c >= '0' && c <= '9') return font_digits_5x7[c - '0'];
	
	
	if (c == 'm') return font_m_lower_5x7;
	if (c == 'V' || c == 'v') return font_v_upper_5x7;
	if (c == 'u') return font_u_lower_5x7;
	if (c == 's') return font_s_lower_5x7;

	switch(c) {
		case '.': return f_dot;
		case ':': return f_col;
		case '=': return f_eq;
		case 'a': return f_a;
		case 'f': return f_f;
		case 'i': return f_i;
		case 'K': return f_K;
		case 'H': return f_H;
		case 'z': case 'Z': return f_z;
		case 'n': return f_n;
		case 'P': return f_P;
		case 'X': return f_X;
		case 'T': return f_T;
		case 'R': return f_R;
		case 'I': return f_I;
		case 'G': return f_G;
		case 'W': return f_W;
		case 'A': return f_A;
		case 'D': return f_D;
		case '%': return f_pe;
		case 'E': return f_E;
		case 'L': return f_L;
		case 'N': return f_N;
		case 'Q': return f_Q;
		case 'Y': return f_Y;
		case 'F': return f_F;
		case 169: case '@': return f_copy; /* for Symbol © */
		case 'C': return f_C;
		case 'M': return f_M;
		case 'O': return f_O;
		case 'S': return f_S;
		case 'c': return f_c;
		case 'g': return f_g;
		case 'h': return f_h;
		case 'r': return f_r;
		case 't': return f_t;
		case 'y': return f_y;
		case 'o': return f_o;
		case 'p': return f_p;
		case '-': return f_minus;
		case 'U': return f_U;
		case 'u': return f_u;
		case '+': return f_plus;
		default:  return font_space_5x7; /* If unknown, draw a space */
	}
}

void drv_ssd1289_fill_rect_phys(uint16_t x,
                                uint16_t y,
                                uint16_t w,
                                uint16_t h,
                                uint16_t color)
{
    uint16_t xx;
    uint16_t yy;

    for (yy = 0u; yy < h; yy++)
    {
        if ((y + yy) >= 240u)
        {
            break;
        }

        for (xx = 0u; xx < w; xx++)
        {
            if ((x + xx) >= 320u)
            {
                break;
            }

            drv_ssd1289_draw_pixel_phys(x + xx, y + yy, color);
        }
    }
}

static uint8_t drv_ssd1289_text_len(const char *text)
{
    uint8_t len = 0u;

    while (text[len] != '\0')
    {
        len++;
    }

    return len;
}

static void drv_ssd1289_draw_scaled_pixel(uint16_t x,
                                          uint16_t y,
                                          uint16_t color)
{
    uint8_t sx;
    uint8_t sy;

    for (sy = 0u; sy < LCD_TEXT_SCALE; sy++)
    {
        for (sx = 0u; sx < LCD_TEXT_SCALE; sx++)
        {
            drv_ssd1289_draw_pixel_phys(x + sx, y + sy, color);
        }
    }
}

static void drv_ssd1289_draw_text_pixel(uint16_t x,
                                        uint16_t y,
                                        uint16_t color)
{
    /*
     * Normal pixel.
     */
    drv_ssd1289_draw_scaled_pixel(x, y, color);

#if LCD_TEXT_BOLD
    /*
     * Slight horizontal thickening.
     * This makes the text a bit larger/stronger without using scale 2.
     */
    if (x < 319u)
    {
        drv_ssd1289_draw_scaled_pixel(x + 1u, y, color);
    }
#endif
}

static void drv_ssd1289_draw_char_small(uint16_t x,
                                        uint16_t y,
                                        char c,
                                        uint16_t fg_color,
                                        uint16_t bg_color)
{
    uint8_t col;
    uint8_t row;
    const uint8_t *bitmap;

    bitmap = drv_ssd1289_get_char_bitmap(c);

    /*
     * Characters are drawn normally.
     * Do not rotate individual digits anymore.
     */
    for (col = 0u; col < 5u; col++)
    {
        for (row = 0u; row < 7u; row++)
        {
            uint16_t px;
            uint16_t py;

            px = (uint16_t)(x + ((uint16_t)col * LCD_TEXT_SCALE));
            py = (uint16_t)(y + ((uint16_t)row * LCD_TEXT_SCALE));

            if (bitmap[col] & (1u << row))
            {
                drv_ssd1289_draw_text_pixel(px, py, fg_color);
            }
            else
            {
                drv_ssd1289_draw_scaled_pixel(px, py, bg_color);
            }
        }
    }

    /*
     * Empty columns between characters.
     * Slightly wider than before to keep bold text readable.
     */
    for (row = 0u; row < 7u; row++)
    {
        drv_ssd1289_draw_scaled_pixel((uint16_t)(x + 5u),
                                      (uint16_t)(y + row),
                                      bg_color);

        drv_ssd1289_draw_scaled_pixel((uint16_t)(x + 6u),
                                      (uint16_t)(y + row),
                                      bg_color);
    }
}

void drv_ssd1289_draw_text_small(uint16_t x,
                                 uint16_t y,
                                 const char *text,
                                 uint16_t fg_color,
                                 uint16_t bg_color)
{
    uint8_t i;
    uint8_t len;
    uint16_t char_step;

    len = drv_ssd1289_text_len(text);

    /*
     * Step = 7 px to keep a small gap between slightly bold characters.
     */
    char_step = 7u;

#if LCD_TEXT_REVERSE_STRING
    /*
     * Reverse only the character order.
     * Individual characters stay upright.
     */
    for (i = 0u; i < len; i++)
    {
        char c = text[len - 1u - i];

        drv_ssd1289_draw_char_small((uint16_t)(x + ((uint16_t)i * char_step)),
                                    y,
                                    c,
                                    fg_color,
                                    bg_color);
    }
#else
    for (i = 0u; i < len; i++)
    {
        drv_ssd1289_draw_char_small((uint16_t)(x + ((uint16_t)i * char_step)),
                                    y,
                                    text[i],
                                    fg_color,
                                    bg_color);
    }
#endif
}

void drv_ssd1289_draw_vdiv_label(uint16_t mv_per_div)
{
    char text[8];

    /*
     * Stable label area at bottom-left.
     */
    drv_ssd1289_fill_rect_phys(0u, 228u, 95u, 12u, LCD_COLOR_BLACK);

    /*
     * Labels:
     * 250 mV
     * 500 mV
     * 1 V
     * 2 V
     * 3 V
     */
    if (mv_per_div == 250u)
    {
        text[0] = '2';
        text[1] = '5';
        text[2] = '0';
        text[3] = ' ';
        text[4] = 'm';
        text[5] = 'V';
        text[6] = '\0';
    }
    else if (mv_per_div == 500u)
    {
        text[0] = '5';
        text[1] = '0';
        text[2] = '0';
        text[3] = ' ';
        text[4] = 'm';
        text[5] = 'V';
        text[6] = '\0';
    }
    else if (mv_per_div == 1000u)
    {
        text[0] = '1';
        text[1] = ' ';
        text[2] = 'V';
        text[3] = '\0';
    }
    else if (mv_per_div == 2000u)
    {
        text[0] = '2';
        text[1] = ' ';
        text[2] = 'V';
        text[3] = '\0';
    }
    else
    {
        text[0] = '3';
        text[1] = ' ';
        text[2] = 'V';
        text[3] = '\0';
    }

    drv_ssd1289_draw_text_small(4u,
                                230u,
                                text,
                                LCD_COLOR_YELLOW,
                                LCD_COLOR_BLACK);
}

void drv_ssd1289_draw_tdiv_label(uint16_t time_div_us)
{
    char text[8];

    /*
     * Time/div label placed next to V/div label.
     */
    drv_ssd1289_fill_rect_phys(70u, 228u, 80u, 12u, LCD_COLOR_BLACK);

    /*
     * Labels:
     * 25 us
     * 50 us
     * 100 us
     * 200 us
     */
    if (time_div_us == 25u)
    {
        text[0] = '2';
        text[1] = '5';
        text[2] = ' ';
        text[3] = 'u';
        text[4] = 's';
        text[5] = '\0';
    }
    else if (time_div_us == 50u)
    {
        text[0] = '5';
        text[1] = '0';
        text[2] = ' ';
        text[3] = 'u';
        text[4] = 's';
        text[5] = '\0';
    }
    else if (time_div_us == 100u)
    {
        text[0] = '1';
        text[1] = '0';
        text[2] = '0';
        text[3] = ' ';
        text[4] = 'u';
        text[5] = 's';
        text[6] = '\0';
    }
    else
    {
        text[0] = '2';
        text[1] = '0';
        text[2] = '0';
        text[3] = ' ';
        text[4] = 'u';
        text[5] = 's';
        text[6] = '\0';
    }

    drv_ssd1289_draw_text_small(70u,
                                230u,
                                text,
                                LCD_COLOR_YELLOW,
                                LCD_COLOR_BLACK);
}