/********************************************************/
/* Projekt: XMEGA-Messplattform (XMP)                   */
/* Datei:   svc_spectrum_renderer.c                     */
/* CPU:     ATxmega128A1                                */
/* Datum:   19.05.2026                                  */
/* Zweck:   Spectrum renderer for FFT analyzer          */
/********************************************************/

#include <stdint.h>

#include "drv_lcd_ssd1289.h"
#include "svc_freq_fft.h"
#include "svc_spectrum_renderer.h"
#include "config.h"
#include "app_home_button.h"

/********************************************************/
/* Colors definition                                    */
/********************************************************/

#ifndef LCD_COLOR_DARK_YELLOW
#define LCD_COLOR_DARK_YELLOW 0x8400
#endif

#ifndef LCD_COLOR_CYAN
#define LCD_COLOR_CYAN 0x07FF
#endif

/********************************************************/
/* Display settings                                     */
/********************************************************/

#define SPEC_X0              0u
#define SPEC_Y0              50u
#define SPEC_W               320u
#define SPEC_H               165u

#define SPEC_MAX_BINS_DRAWN  80u
#define SPEC_FIRST_BIN       1u
#define SPEC_PEAK_MIN_DISTANCE  2u

/********************************************************/
/* Internal state                                       */
/********************************************************/

static uint8_t s_spectrum_initialized = 0u;

/********************************************************/
/* Internal helpers : string building                   */
/********************************************************/

static uint8_t str_append_char(char *dst, uint8_t pos, char c)
{
	dst[pos] = c;
	return (uint8_t)(pos + 1u);
}

static uint8_t str_append_u32(char *dst, uint8_t pos, uint32_t value)
{
	char tmp[10];
	uint8_t i = 0u;

	if (value == 0u)
	{
		dst[pos] = '0';
		return (uint8_t)(pos + 1u);
	}

	while ((value > 0u) && (i < 10u))
	{
		tmp[i] = (char)('0' + (value % 10u));
		value /= 10u;
		i++;
	}

	while (i > 0u)
	{
		i--;
		dst[pos] = tmp[i];
		pos++;
	}

	return pos;
}

static uint8_t str_append_ram(char *dst, uint8_t pos, const char *src)
{
	while (*src != '\0')
	{
		dst[pos] = *src;
		pos++;
		src++;
	}

	return pos;
}

static void spectrum_make_peak_text(char *dst,
char peak_number,
uint32_t freq_hz,
uint32_t pct)
{
	uint8_t pos = 0u;

	pos = str_append_char(dst, pos, 'P');
	pos = str_append_char(dst, pos, peak_number);
	pos = str_append_char(dst, pos, ':');
	pos = str_append_u32(dst, pos, freq_hz);
	pos = str_append_ram(dst, pos, "Hz ");
	pos = str_append_u32(dst, pos, pct);
	pos = str_append_char(dst, pos, '%');

	dst[pos] = '\0';
}

static void spectrum_make_fdiv_text(char *dst, uint32_t fdiv_hz)
{
	uint8_t pos = 0u;

	pos = str_append_ram(dst, pos, "FDIV:");
	pos = str_append_u32(dst, pos, fdiv_hz);
	pos = str_append_ram(dst, pos, "Hz");

	dst[pos] = '\0';
}

static void spectrum_make_freq_text(char *dst, uint32_t freq_hz)
{
	uint8_t pos = 0u;

	pos = str_append_u32(dst, pos, freq_hz);
	pos = str_append_ram(dst, pos, "Hz");

	dst[pos] = '\0';
}

/********************************************************/
/* Internal helpers : drawing                           */
/********************************************************/

static void spectrum_draw_vline(uint16_t x,
uint16_t y0,
uint16_t y1,
uint16_t color)
{
	uint16_t y;
	uint16_t tmp;

	if (x >= 320u)
	{
		return;
	}

	if (y0 > y1)
	{
		tmp = y0;
		y0 = y1;
		y1 = tmp;
	}

	if (y1 >= 240u)
	{
		y1 = 239u;
	}

	for (y = y0; y <= y1; y++)
	{
		drv_ssd1289_draw_pixel_phys(x, y, color);
	}
}

static void spectrum_draw_hline(uint16_t x0,
uint16_t x1,
uint16_t y,
uint16_t color)
{
	uint16_t x;
	uint16_t tmp;

	if (y >= 240u)
	{
		return;
	}

	if (x0 > x1)
	{
		tmp = x0;
		x0 = x1;
		x1 = tmp;
	}

	if (x1 >= 320u)
	{
		x1 = 319u;
	}

	for (x = x0; x <= x1; x++)
	{
		drv_ssd1289_draw_pixel_phys(x, y, color);
	}
}

static void spectrum_fill_rect(uint16_t x,
uint16_t y,
uint16_t w,
uint16_t h,
uint16_t color)
{
	uint16_t yy;
	uint16_t xx;

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

			drv_ssd1289_draw_pixel_phys((uint16_t)(x + xx),
			(uint16_t)(y + yy),
			color);
		}
	}
}

static void spectrum_draw_grid_10x8(void)
{
	uint8_t i;
	uint16_t x;
	uint16_t y;

	for (i = 0u; i <= 10u; i++)
	{
		x = (uint16_t)(SPEC_X0 + ((uint16_t)i * (SPEC_W / 10u)));

		if (x >= 320u)
		{
			x = 319u;
		}

		spectrum_draw_vline(x,
		SPEC_Y0,
		(uint16_t)(SPEC_Y0 + SPEC_H),
		LCD_COLOR_GRAY);
	}

	for (i = 0u; i <= 8u; i++)
	{
		y = (uint16_t)(SPEC_Y0 + ((uint16_t)i * (SPEC_H / 8u)));

		if (y >= 240u)
		{
			y = 239u;
		}

		spectrum_draw_hline(SPEC_X0,
		(uint16_t)(SPEC_X0 + SPEC_W - 1u),
		y,
		LCD_COLOR_GRAY);
	}
}

/********************************************************/
/* Internal helpers : maths and logic                   */
/********************************************************/

static uint32_t spectrum_find_max_mag(uint16_t first_bin,
uint16_t bin_count)
{
	uint16_t i;
	uint16_t bin;
	uint32_t mag;
	uint32_t max_mag = 1u;

	for (i = 0u; i < bin_count; i++)
	{
		bin = (uint16_t)(first_bin + i);
		mag = svc_freq_fft_get_bin_magnitude(bin);

		if (mag > max_mag)
		{
			max_mag = mag;
		}
	}

	return max_mag;
}

static uint8_t spectrum_bin_is_far_enough(uint16_t bin,
uint16_t p1,
uint16_t p2,
uint16_t p3)
{
	uint16_t d;

	if (p1 != 0u)
	{
		d = (bin > p1) ? (uint16_t)(bin - p1) : (uint16_t)(p1 - bin);
		if (d < SPEC_PEAK_MIN_DISTANCE)
		{
			return 0u;
		}
	}

	if (p2 != 0u)
	{
		d = (bin > p2) ? (uint16_t)(bin - p2) : (uint16_t)(p2 - bin);
		if (d < SPEC_PEAK_MIN_DISTANCE)
		{
			return 0u;
		}
	}

	if (p3 != 0u)
	{
		d = (bin > p3) ? (uint16_t)(bin - p3) : (uint16_t)(p3 - bin);
		if (d < SPEC_PEAK_MIN_DISTANCE)
		{
			return 0u;
		}
	}

	return 1u;
}

static void spectrum_find_top3(uint16_t first_bin,
uint16_t bin_count,
uint16_t *p1_bin,
uint32_t *p1_mag,
uint16_t *p2_bin,
uint32_t *p2_mag,
uint16_t *p3_bin,
uint32_t *p3_mag)
{
	uint8_t pass;
	uint16_t i;

	*p1_bin = 0u;
	*p2_bin = 0u;
	*p3_bin = 0u;

	*p1_mag = 0u;
	*p2_mag = 0u;
	*p3_mag = 0u;

	for (pass = 0u; pass < 3u; pass++)
	{
		uint16_t best_bin = 0u;
		uint32_t best_mag = 0u;

		for (i = 0u; i < bin_count; i++)
		{
			uint16_t bin;
			uint32_t mag;

			bin = (uint16_t)(first_bin + i);
			mag = svc_freq_fft_get_bin_magnitude(bin);

			if (!spectrum_bin_is_far_enough(bin, *p1_bin, *p2_bin, *p3_bin))
			{
				continue;
			}

			if (mag > best_mag)
			{
				best_mag = mag;
				best_bin = bin;
			}
		}

		if (pass == 0u)
		{
			*p1_bin = best_bin;
			*p1_mag = best_mag;
		}
		else if (pass == 1u)
		{
			*p2_bin = best_bin;
			*p2_mag = best_mag;
		}
		else
		{
			*p3_bin = best_bin;
			*p3_mag = best_mag;
		}
	}
}

/********************************************************/
/* Header and labels                                    */
/********************************************************/

static void spectrum_draw_header(const freq_fft_exec_t *fft_result,
uint16_t p1_bin,
uint32_t p1_mag,
uint16_t p2_bin,
uint32_t p2_mag,
uint16_t p3_bin,
uint32_t p3_mag)
{
	char text[24];

	uint32_t p1_freq;
	uint32_t p2_freq;
	uint32_t p3_freq;

	uint32_t p1_pct = 0u;
	uint32_t p2_pct = 0u;
	uint32_t p3_pct = 0u;

	if (p1_mag > 0u)
	{
		p1_pct = 100u;
		p2_pct = (p2_mag * 100u) / p1_mag;
		p3_pct = (p3_mag * 100u) / p1_mag;
	}

	p1_freq = ((uint32_t)p1_bin * (uint32_t)fft_result->fs_eff_hz) /
	(uint32_t)SVC_FREQ_FFT_N;

	p2_freq = ((uint32_t)p2_bin * (uint32_t)fft_result->fs_eff_hz) /
	(uint32_t)SVC_FREQ_FFT_N;

	p3_freq = ((uint32_t)p3_bin * (uint32_t)fft_result->fs_eff_hz) /
	(uint32_t)SVC_FREQ_FFT_N;

	spectrum_fill_rect(0u, 0u, 320u, 45u, LCD_COLOR_BLACK);

	drv_ssd1289_draw_text_small(5u,
	5u,
	"FREQ ANALYZER",
	LCD_COLOR_WHITE,
	LCD_COLOR_BLACK);

	spectrum_make_peak_text(text, '1', p1_freq, p1_pct);
	drv_ssd1289_draw_text_small(5u,
	17u,
	text,
	LCD_COLOR_RED,
	LCD_COLOR_BLACK);

	spectrum_make_peak_text(text, '2', p2_freq, p2_pct);
	drv_ssd1289_draw_text_small(110u,
	17u,
	text,
	LCD_COLOR_GREEN,
	LCD_COLOR_BLACK);

	spectrum_make_peak_text(text, '3', p3_freq, p3_pct);
	drv_ssd1289_draw_text_small(215u,
	17u,
	text,
	LCD_COLOR_CYAN,
	LCD_COLOR_BLACK);

	spectrum_make_fdiv_text(text,
	((uint32_t)SPEC_MAX_BINS_DRAWN *
	(uint32_t)fft_result->bin_width_hz) / 10u);

	drv_ssd1289_draw_text_small(5u,
	32u,
	text,
	LCD_COLOR_GRAY,
	LCD_COLOR_BLACK);
}

static void spectrum_draw_bottom_labels(const freq_fft_exec_t *fft_result)
{
	char text[16];
	uint32_t f_max;

	spectrum_fill_rect(0u, 220u, 320u, 20u, LCD_COLOR_BLACK);

	drv_ssd1289_draw_text_small(5u,
	230u,
	"0Hz",
	LCD_COLOR_WHITE,
	LCD_COLOR_BLACK);

	f_max = (uint32_t)SPEC_MAX_BINS_DRAWN *
	(uint32_t)fft_result->bin_width_hz;

	spectrum_make_freq_text(text, f_max);

	drv_ssd1289_draw_text_small(245u,
	230u,
	text,
	LCD_COLOR_WHITE,
	LCD_COLOR_BLACK);
}

/********************************************************/
/* Public functions                                     */
/********************************************************/

void svc_spectrum_renderer_reset(void)
{
	s_spectrum_initialized = 0u;
}

void svc_spectrum_renderer_draw_basic(const freq_fft_exec_t *fft_result)
{
	app_home_button_draw();
	uint16_t i;
	uint32_t max_mag;

	uint16_t p1_bin;
	uint16_t p2_bin;
	uint16_t p3_bin;

	uint32_t p1_mag;
	uint32_t p2_mag;
	uint32_t p3_mag;

	if (fft_result == 0 || !fft_result->valid)
	{
		return;
	}

	if (!s_spectrum_initialized)
	{
		drv_ssd1289_fill_screen(LCD_COLOR_BLACK);
		s_spectrum_initialized = 1u;
	}

	spectrum_find_top3(SPEC_FIRST_BIN,
	SPEC_MAX_BINS_DRAWN,
	&p1_bin,
	&p1_mag,
	&p2_bin,
	&p2_mag,
	&p3_bin,
	&p3_mag);

	spectrum_draw_header(fft_result,
	p1_bin,
	p1_mag,
	p2_bin,
	p2_mag,
	p3_bin,
	p3_mag);

	spectrum_fill_rect(SPEC_X0,
	SPEC_Y0,
	SPEC_W,
	(uint16_t)(SPEC_H + 1u),
	LCD_COLOR_BLACK);

	spectrum_draw_grid_10x8();

	max_mag = spectrum_find_max_mag(SPEC_FIRST_BIN,
	SPEC_MAX_BINS_DRAWN);

	for (i = 0u; i < SPEC_MAX_BINS_DRAWN; i++)
	{
		uint16_t bin;
		uint32_t mag;
		uint16_t bar_h;
		uint16_t x;
		uint16_t y_base;
		uint16_t y_top;
		uint16_t marker_color;
		uint16_t marker_y;
		uint16_t marker_x0;
		uint16_t marker_x1;

		bin = (uint16_t)(SPEC_FIRST_BIN + i);
		mag = svc_freq_fft_get_bin_magnitude(bin);

		bar_h = (uint16_t)((mag * (uint32_t)(SPEC_H - 8u)) / max_mag);

		if (bar_h > (SPEC_H - 8u))
		{
			bar_h = (uint16_t)(SPEC_H - 8u);
		}

		x = (uint16_t)(SPEC_X0 + (i * 4u) + 1u);
		y_base = (uint16_t)(SPEC_Y0 + SPEC_H);
		y_top = (uint16_t)(y_base - bar_h);

		if (bar_h > 1u)
		{
			spectrum_draw_vline(x,
			(uint16_t)(y_top + 1u),
			y_base,
			LCD_COLOR_DARK_YELLOW);

			spectrum_draw_vline((uint16_t)(x + 1u),
			(uint16_t)(y_top + 1u),
			y_base,
			LCD_COLOR_DARK_YELLOW);
		}

		drv_ssd1289_draw_pixel_phys(x,
		y_top,
		LCD_COLOR_YELLOW);

		drv_ssd1289_draw_pixel_phys((uint16_t)(x + 1u),
		y_top,
		LCD_COLOR_YELLOW);

		marker_color = LCD_COLOR_BLACK;

		if ((bin == p1_bin) && (p1_mag > 0u))
		{
			marker_color = LCD_COLOR_RED;
		}
		else if ((bin == p2_bin) && (p2_mag > 0u))
		{
			marker_color = LCD_COLOR_GREEN;
		}
		else if ((bin == p3_bin) && (p3_mag > 0u))
		{
			marker_color = LCD_COLOR_CYAN;
		}

		if (marker_color != LCD_COLOR_BLACK)
		{
			marker_y = (y_top > (SPEC_Y0 + 5u)) ?
			(uint16_t)(y_top - 5u) :
			SPEC_Y0;

			if (x >= 2u)
			{
				marker_x0 = (uint16_t)(x - 2u);
			}
			else
			{
				marker_x0 = 0u;
			}

			marker_x1 = (uint16_t)(x + 3u);

			if (marker_x1 >= 320u)
			{
				marker_x1 = 319u;
			}

			spectrum_draw_vline(x,
			marker_y,
			(uint16_t)(marker_y + 3u),
			marker_color);

			spectrum_draw_hline(marker_x0,
			marker_x1,
			marker_y,
			marker_color);
		}
	}

	spectrum_draw_bottom_labels(fft_result);
}
