/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   main.c                                   */
/*    CPU:     ATxmega128A1                             */
/*    Autor:   Lauris Taguetieu                         */
/*    Datum:   05.05.26                                 */
/*    Version: 1.9                                      */
/********************************************************/

#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h> /* for dtostrf */
#include <stdio.h>  /* for sprintf */

#include "config.h"
#include "drv_clock.h"
#include "drv_gpio.h"
#include "drv_ds1803.h"
#include "drv_ds1803_gen.h"
#include "drv_timer_ui.h"
#include "drv_adc.h"
#include "drv_dac.h"
#include "drv_timer_gen.h"
#include "drv_touch_ads7843.h"
#include "drv_lcd_ssd1289.h"
#include "drv_scope_controls.h"
#include "drv_timer_sample.h"

#include "svc_acq.h"
#include "svc_renderer.h"
#include "svc_trigger.h"
#include "svc_freq_fft.h"
#include "svc_spectrum_renderer.h"
#include "svc_siggen.h"

#include "app_mode.h"
#include "app_menu.h"
#include "app_siggen.h"


/* Debug variables */
volatile uint8_t  g_pot_raw = 0;
volatile uint16_t g_trigger_line_y = 120u;
volatile int32_t  g_trigger_level_mv = 0;
volatile uint8_t  g_trigger_raw = 0;
volatile int16_t  g_last_trigger_index = -1;

volatile uint16_t g_display_start_index = 0u;
volatile uint16_t g_last_good_display_start = 0u;
volatile uint8_t  g_trigger_locked = 0u;
volatile uint32_t g_trigger_hold_counter = 0u;

volatile uint16_t g_pretrigger_samples = 0u;
volatile uint16_t g_trigger_target_index = 0u;

volatile uint16_t g_mv_per_div = 250u;
volatile uint8_t  g_vdiv_index = 0u;
volatile uint16_t g_vdiv_press_count = 0u;

volatile uint16_t g_time_div_us = 25u;
volatile uint8_t  g_tdiv_index = 0u;
volatile uint16_t g_tdiv_press_count = 0u;

volatile uint16_t g_view_samples = 500u;
volatile uint32_t g_frame_counter = 0;
volatile uint32_t g_loop_counter = 0;

/* --- Phase 10 menu / touch debug variables --- */
volatile uint8_t  g_app_mode = APP_MODE_MENU;
volatile uint16_t g_touch_last_x = 0u;
volatile uint16_t g_touch_last_y = 0u;
volatile uint8_t  g_menu_touch_locked = 0u;

/* --- Frequency analyzer debug / redraw variables --- */
volatile uint16_t g_spectrum_draw_counter = 0u;

/* --- UI Redraw Control Flag --- */
volatile uint8_t g_force_full_redraw = 1u;

/* DAC debug variables*/
volatile uint8_t  g_debug_gen_dac_initialized = 0u;
volatile uint16_t g_debug_gen_dac_code = 0u;
volatile uint16_t g_debug_gen_dac_mv = 0u;

#define SPECTRUM_DRAW_EVERY_N_FRAMES  100u



static app_mode_t app_wait_for_start_menu_selection(void)
{
    uint16_t tx = 0u;
    uint16_t ty = 0u;
    app_mode_t selected_mode = APP_MODE_MENU;

    app_mode_init();
    app_mode_set(APP_MODE_MENU);
    g_app_mode = (uint8_t)APP_MODE_MENU;
    g_menu_touch_locked = 0u;

    app_menu_force_redraw();
    app_menu_draw();

    while (1)
    {
        /*
         * Touch is used only here, in the start menu.
         * After a mode is selected, touch is no longer evaluated
         * by the oscilloscope mode.
         */
        if (!drv_touch_ads7843_read_screen(&tx, &ty))
        {
            g_menu_touch_locked = 0u;
            continue;
        }

        g_touch_last_x = tx;
        g_touch_last_y = ty;

        /*
         * One action per touch press.
         */
        if (g_menu_touch_locked)
        {
            continue;
        }

        g_menu_touch_locked = 1u;

        if (app_menu_get_mode_from_touch(tx, ty, &selected_mode))
        {
            app_mode_set(selected_mode);
            g_app_mode = (uint8_t)selected_mode;

            /*
             * Small delay before entering the selected mode.
             */
            _delay_ms(120);
            return selected_mode;
        }
    }
}

static void app_run_frequency_analyzer_mode(void)
{
    uint8_t capture_busy = 0u;

    /*
     * Frequency Analyzer mode uses the same validated acquisition base
     * as the oscilloscope, but renders the FFT spectrum instead of the
     * time-domain waveform. No M button and no touch handling are active
     * inside this mode.
     */
    drv_clock_init_32mhz();
    drv_gpio_init();
    drv_ds1803_init();
    svc_hardware_offset_apply(0x69, 0xFF);
    drv_timer_ui_init_1ms();

    drv_ssd1289_init();
    drv_adc_init();

    svc_acq_init();
    drv_timer_sample_init(XMP_FS_HZ);

    svc_spectrum_renderer_reset();
    g_spectrum_draw_counter = SPECTRUM_DRAW_EVERY_N_FRAMES;

    drv_ssd1289_fill_screen(LCD_COLOR_BLACK);
    drv_ssd1289_draw_text_small(70u,
                                105u,
                                "FREQ ANALYZER",
                                LCD_COLOR_GREEN,
                                LCD_COLOR_BLACK);

    svc_acq_start();
    drv_timer_sample_start();
    capture_busy = 1u;

    while (1)
    {
        drv_timer_ui_take_ticks();

        if (capture_busy && svc_acq_done())
        {
            const volatile uint8_t *buf;
            freq_fft_prep_t fft_prep;
            freq_fft_exec_t fft_exec;

            drv_timer_sample_stop();
            capture_busy = 0u;

            buf = svc_acq_get_ch0_raw();

            /*
             * RAM optimization validated in Phase 9B-2:
             * The DMA is stopped here, so the frozen ADC buffer can be
             * reused temporarily as the FFT workspace.
             */
            svc_freq_fft_bind_buffer((int16_t *)(uintptr_t)buf,
                                     SVC_FREQ_FFT_N);

            svc_freq_fft_prepare_1024(buf,
                                      XMP_ACQ_N,
                                      0u,
                                      XMP_FS_HZ,
                                      &fft_prep);

            if (fft_prep.valid)
            {
                svc_freq_fft_forward_1024(&fft_exec);

                g_spectrum_draw_counter++;

                if (g_spectrum_draw_counter >= SPECTRUM_DRAW_EVERY_N_FRAMES)
                {
                    g_spectrum_draw_counter = 0u;

                    if (fft_exec.valid)
                    {
                        svc_spectrum_renderer_draw_basic(&fft_exec);
                    }
                }
            }

            svc_acq_start();
            drv_timer_sample_start();
            capture_busy = 1u;
        }
    }
}

static uint32_t app_gen_next_freq_step(uint32_t current_step_hz)
{
    if (current_step_hz == 1UL)
    {
        return 10UL;
    }

    if (current_step_hz == 10UL)
    {
        return 100UL;
    }

    if (current_step_hz == 100UL)
    {
        return 1000UL;
    }

    if (current_step_hz == 1000UL)
    {
        return 10000UL;
    }

    if (current_step_hz == 10000UL)
    {
        return 100000UL;
    }

    return 1UL;
}

static uint16_t app_gen_next_vpp_step(uint16_t current_step_mv)
{
    /*
     * VPP step sequence:
     * 50 mV -> 100 mV -> 500 mV -> 1000 mV -> 50 mV
     */
    if (current_step_mv == 50u)
    {
        return 100u;
    }

    if (current_step_mv == 100u)
    {
        return 500u;
    }

    if (current_step_mv == 500u)
    {
        return 1000u;
    }

    return 50u;
}

static int16_t app_gen_next_offset_step(int16_t current_step_mv)
{
    /*
     * Offset step sequence:
     * 50 mV -> 100 mV -> 500 mV -> 1000 mV -> 50 mV
     */
    if (current_step_mv == 50)
    {
        return 100;
    }

    if (current_step_mv == 100)
    {
        return 500;
    }

    if (current_step_mv == 500)
    {
        return 1000;
    }

    return 50;
}

static uint8_t app_gen_next_duty_step(uint8_t current_step_percent)
{
    if (current_step_percent == 1u)
    {
        return 5u;
    }

    if (current_step_percent == 5u)
    {
        return 10u;
    }

    return 1u;
}

static siggen_waveform_t app_gen_to_service_waveform(app_siggen_waveform_t waveform)
{
    if (waveform == APP_SIGGEN_WAVE_TRIANGLE)
    {
        return SIGGEN_WAVE_TRIANGLE;
    }

    if (waveform == APP_SIGGEN_WAVE_SINE)
    {
        return SIGGEN_WAVE_SINE;
    }

    return SIGGEN_WAVE_SQUARE;
}

static void app_gen_clamp_view(app_siggen_view_t *view)
{
    if (view == 0)
    {
        return;
    }

    if (view->frequency_hz < 1UL)
    {
        view->frequency_hz = 1UL;
    }

    /*
     * All generator waveforms are limited to 100 kHz.
     *
     * Note:
     * Sine and triangle above the clean waveform range are available as
     * test/output modes, but the waveform shape becomes coarse at high
     * frequencies because only a few samples per period are possible.
     */
    if (view->frequency_hz > 100000UL)
    {
        view->frequency_hz = 100000UL;
    }

    if (view->vpp_mv < 100u)
    {
        view->vpp_mv = 100u;
    }

    if (view->vpp_mv > 6000u)
    {
        view->vpp_mv = 6000u;
    }

    if (view->duty_percent < 5u)
    {
        view->duty_percent = 5u;
    }

    if (view->duty_percent > 95u)
    {
        view->duty_percent = 95u;
    }

    /*
     * Offset is not limited by VPP.
     *
     * This is required for modern signal-generator behavior:
     *
     * Vlow  = Offset - VPP / 2
     * Vhigh = Offset + VPP / 2
     *
     * Examples:
     * 0...5 V   -> VPP = 5000 mV, Offset = +2500 mV
     * 0...3.3 V -> VPP = 3300 mV, Offset = +1650 mV
     *
     * Positive offset up to +5000 mV is allowed for unipolar signals.
     * Negative offset down to -3200 mV is allowed for the measured
     * bipolar profile.
     */
    if (view->offset_mv > 5000)
    {
        view->offset_mv = 5000;
    }

    if (view->offset_mv < -3200)
    {
        view->offset_mv = -3200;
    }
}

static uint8_t app_gen_get_new_touch_press(uint16_t *x,
                                           uint16_t *y)
{
    static uint8_t s_touch_was_down = 0u;
    static uint16_t s_release_counter = 0u;

    uint16_t tx;
    uint16_t ty;

    if ((x == 0) || (y == 0))
    {
        return 0u;
    }

    /*
     * If the screen is not pressed, release the lock.
     */
    if (!drv_touch_ads7843_is_pressed())
    {
        s_touch_was_down = 0u;
        s_release_counter = 0u;
        return 0u;
    }

    /*
     * If the touch was already handled, wait for release.
     * The release counter prevents a permanent lock if PENIRQ stays active.
     */
    if (s_touch_was_down)
    {
        s_release_counter++;

        if (s_release_counter > 300u)
        {
            /*
             * Safety unlock after a long press or unstable PENIRQ state.
             */
            s_touch_was_down = 0u;
            s_release_counter = 0u;
        }

        return 0u;
    }

    if (!drv_touch_ads7843_read_screen(&tx, &ty))
    {
        return 0u;
    }

    /*
     * Reject obviously invalid coordinates.
     */
    if ((tx > 319u) || (ty > 239u))
    {
        return 0u;
    }

    *x = tx;
    *y = ty;

    s_touch_was_down = 1u;
    s_release_counter = 0u;

    /*
     * Small debounce delay.
     */
    _delay_ms(20);

    return 1u;
}

static void app_gen_wait_touch_release(uint16_t timeout_ms)
{
    uint16_t elapsed_ms = 0u;

    /*
     * Wait until the user releases the touch screen.
     * A timeout prevents a permanent lock if PENIRQ stays active.
     */
    while (drv_touch_ads7843_is_pressed() && (elapsed_ms < timeout_ms))
    {
        _delay_ms(5);
        elapsed_ms = (uint16_t)(elapsed_ms + 5u);
    }
}

static uint8_t app_gen_configure_without_start(app_siggen_view_t *view)
{
    uint8_t ok;

    if (view == 0)
    {
        return 0u;
    }

    /*
     * Stop the generator before changing timer/DAC configuration.
     * This avoids conflicts between ISR, DMA, touch handling and LCD updates.
     */
    svc_siggen_stop();
    _delay_ms(5);

    app_gen_clamp_view(view);

    ok = svc_siggen_configure(app_gen_to_service_waveform(view->waveform),
                              view->frequency_hz,
                              view->vpp_mv,
                              view->offset_mv,
                              view->duty_percent);

    view->clipping_warning = (ok == 0u) ? 1u : 0u;

    return ok;
}

static void app_gen_start_after_ui_update(app_siggen_view_t *view,
                                          uint8_t config_ok)
{
    if (view == 0)
    {
        return;
    }

    /*
     * Restart only after the display has been updated.
     */
    if (view->output_on && config_ok)
    {
        svc_siggen_start();
    }
    else
    {
        svc_siggen_stop();
    }
}

static void app_gen_switch_waveform_safely(app_siggen_view_t *view,
                                           app_siggen_waveform_t new_waveform)
{
    uint8_t config_ok;

    if (view == 0)
    {
        return;
    }

    /*
     * Do nothing if the selected waveform is already active.
     */
    if (view->waveform == new_waveform)
    {
        app_gen_wait_touch_release(300u);
        return;
    }

    /*
     * Block ultra-fast waveform jumping.
     */
    svc_siggen_stop();
    _delay_ms(20);

    view->waveform = new_waveform;
    view->edit_target = APP_SIGGEN_EDIT_NONE;
    view->clipping_warning = 0u;

    /*
     * Configure the generator, but do not restart it yet.
     */
    config_ok = app_gen_configure_without_start(view);

    /*
     * Update only the areas that can change after a waveform change.
     */
    app_siggen_redraw_waveform_area(view);
    app_siggen_redraw_freq_area(view);
    app_siggen_redraw_vpp_area(view);
    app_siggen_redraw_duty_area(view);
    app_siggen_redraw_bottom_area(view);
    app_siggen_redraw_edit_area(view);

    /*
     * Restart only after the UI update is finished.
     */
    app_gen_start_after_ui_update(view, config_ok);

    /*
     * Wait for release and add a small cooldown.
     * This prevents multiple waveform changes from one physical press.
     */
    app_gen_wait_touch_release(400u);
    _delay_ms(80);
}

static uint8_t app_gen_handle_plus_minus_safely(app_siggen_view_t *view,
                                                uint8_t increase)
{
    if (view == 0)
    {
        return 0u;
    }

    if (view->edit_target == APP_SIGGEN_EDIT_FREQ)
    {
        if (increase)
        {
            view->frequency_hz += view->freq_step_hz;
        }
        else
        {
            if (view->frequency_hz > view->freq_step_hz)
            {
                view->frequency_hz -= view->freq_step_hz;
            }
            else
            {
                view->frequency_hz = 1UL;
            }
        }

        return app_gen_configure_without_start(view);
    }

    if (view->edit_target == APP_SIGGEN_EDIT_VPP)
    {
        if (increase)
        {
            view->vpp_mv = (uint16_t)(view->vpp_mv + view->vpp_step_mv);
        }
        else
        {
            if (view->vpp_mv > view->vpp_step_mv)
            {
                view->vpp_mv = (uint16_t)(view->vpp_mv - view->vpp_step_mv);
            }
            else
            {
                view->vpp_mv = 100u;
            }
        }

        return app_gen_configure_without_start(view);
    }

    if (view->edit_target == APP_SIGGEN_EDIT_OFFSET)
    {
        if (increase)
        {
            view->offset_mv = (int16_t)(view->offset_mv +
                                        view->offset_step_mv);
        }
        else
        {
            view->offset_mv = (int16_t)(view->offset_mv -
                                        view->offset_step_mv);
        }

        return app_gen_configure_without_start(view);
    }

    if (view->edit_target == APP_SIGGEN_EDIT_DUTY)
    {
        if (view->waveform != APP_SIGGEN_WAVE_SQUARE)
        {
            view->clipping_warning = 1u;
            return 0u;
        }

        if (increase)
        {
            view->duty_percent = (uint8_t)(view->duty_percent +
                                           view->duty_step_percent);
        }
        else
        {
            if (view->duty_percent > view->duty_step_percent)
            {
                view->duty_percent = (uint8_t)(view->duty_percent -
                                               view->duty_step_percent);
            }
            else
            {
                view->duty_percent = 5u;
            }
        }

        return app_gen_configure_without_start(view);
    }

    return 0u;
}

static void app_run_gen_placeholder(void)
{
    uint16_t tx = 0u;
    uint16_t ty = 0u;
    uint8_t config_ok;
    app_siggen_touch_action_t action;
    app_siggen_view_t view;

    /*
     * Configure the external analog stage for the generator.
     */
    drv_ds1803_gen_init();
    drv_ds1803_gen_apply_default_max_symmetric();

    /*
     * Initial generator view.
     */
    view.waveform = APP_SIGGEN_WAVE_SQUARE;
    view.frequency_hz = 1000UL;
    view.vpp_mv = 6000u;
    view.offset_mv = 0;
    view.duty_percent = 50u;
    view.output_on = 1u;
    view.clipping_warning = 0u;

    view.edit_target = APP_SIGGEN_EDIT_NONE;
    view.freq_step_hz = 100u;
    view.vpp_step_mv = 50u;
    view.offset_step_mv = 50;
    view.duty_step_percent = 5u;

    /*
     * Configure first, draw the full UI, then start the generator.
     * This avoids running the generator while the full display is drawn.
     */
    config_ok = app_gen_configure_without_start(&view);
    app_siggen_draw(&view);
    app_gen_start_after_ui_update(&view, config_ok);

    while (1)
    {
        drv_timer_ui_take_ticks();

        if (!app_gen_get_new_touch_press(&tx, &ty))
        {
            continue;
        }

        g_touch_last_x = tx;
        g_touch_last_y = ty;

        if (!app_siggen_get_touch_action(tx, ty, &view, &action))
        {
            continue;
        }

        if (action == APP_SIGGEN_TOUCH_WAVE_SQUARE)
        {
            app_gen_switch_waveform_safely(&view,
                                           APP_SIGGEN_WAVE_SQUARE);
        }
        else if (action == APP_SIGGEN_TOUCH_WAVE_TRIANGLE)
        {
            app_gen_switch_waveform_safely(&view,
                                           APP_SIGGEN_WAVE_TRIANGLE);
        }
        else if (action == APP_SIGGEN_TOUCH_WAVE_SINE)
        {
            app_gen_switch_waveform_safely(&view,
                                           APP_SIGGEN_WAVE_SINE);
        }
        else if (action == APP_SIGGEN_TOUCH_PARAM_FREQ)
        {
            view.edit_target = APP_SIGGEN_EDIT_FREQ;
            view.clipping_warning = 0u;

            app_siggen_redraw_edit_area(&view);
        }
        else if (action == APP_SIGGEN_TOUCH_PARAM_VPP)
        {
            view.edit_target = APP_SIGGEN_EDIT_VPP;
            view.clipping_warning = 0u;

            app_siggen_redraw_edit_area(&view);
        }
        else if (action == APP_SIGGEN_TOUCH_PARAM_OFFSET)
        {
            view.edit_target = APP_SIGGEN_EDIT_OFFSET;
            view.clipping_warning = 0u;

            app_siggen_redraw_edit_area(&view);
        }
        else if (action == APP_SIGGEN_TOUCH_PARAM_DUTY)
        {
            if (view.waveform == APP_SIGGEN_WAVE_SQUARE)
            {
                view.edit_target = APP_SIGGEN_EDIT_DUTY;
                view.clipping_warning = 0u;
            }
            else
            {
                view.clipping_warning = 1u;
            }

            app_siggen_redraw_edit_area(&view);
        }
        else if (action == APP_SIGGEN_TOUCH_OUTPUT)
        {
            config_ok = 1u;

            view.output_on = (view.output_on == 0u) ? 1u : 0u;
            view.clipping_warning = 0u;

            if (view.output_on)
            {
                config_ok = app_gen_configure_without_start(&view);
            }
            else
            {
                svc_siggen_stop();
            }

            app_siggen_redraw_bottom_area(&view);
            app_siggen_redraw_edit_area(&view);

            app_gen_start_after_ui_update(&view, config_ok);

            app_gen_wait_touch_release(300u);
            _delay_ms(50);
        }
        else if (action == APP_SIGGEN_TOUCH_STEP)
        {
            if (view.edit_target == APP_SIGGEN_EDIT_FREQ)
            {
                view.freq_step_hz = app_gen_next_freq_step(view.freq_step_hz);
                view.clipping_warning = 0u;
            }
            else if (view.edit_target == APP_SIGGEN_EDIT_VPP)
            {
                view.vpp_step_mv = app_gen_next_vpp_step(view.vpp_step_mv);
                view.clipping_warning = 0u;
            }
            else if (view.edit_target == APP_SIGGEN_EDIT_OFFSET)
            {
                view.offset_step_mv = app_gen_next_offset_step(view.offset_step_mv);
                view.clipping_warning = 0u;
            }
            else if (view.edit_target == APP_SIGGEN_EDIT_DUTY)
            {
                view.duty_step_percent = app_gen_next_duty_step(view.duty_step_percent);
                view.clipping_warning = 0u;
            }
            else
            {
                view.clipping_warning = 1u;
            }

            app_siggen_redraw_edit_area(&view);
        }
        else if (action == APP_SIGGEN_TOUCH_PLUS)
        {
            config_ok = app_gen_handle_plus_minus_safely(&view, 1u);

            if (view.edit_target == APP_SIGGEN_EDIT_FREQ)
            {
                app_siggen_redraw_freq_area(&view);
            }
            else if (view.edit_target == APP_SIGGEN_EDIT_VPP)
            {
                app_siggen_redraw_vpp_area(&view);
                app_siggen_redraw_bottom_area(&view);
            }
            else if (view.edit_target == APP_SIGGEN_EDIT_OFFSET)
            {
                app_siggen_redraw_bottom_area(&view);
            }
            else if (view.edit_target == APP_SIGGEN_EDIT_DUTY)
            {
                app_siggen_redraw_duty_area(&view);
            }

            app_siggen_redraw_edit_area(&view);

            app_gen_start_after_ui_update(&view, config_ok);

            app_gen_wait_touch_release(300u);
            _delay_ms(30);
        }
        else if (action == APP_SIGGEN_TOUCH_MINUS)
        {
            config_ok = app_gen_handle_plus_minus_safely(&view, 0u);

            if (view.edit_target == APP_SIGGEN_EDIT_FREQ)
            {
                app_siggen_redraw_freq_area(&view);
            }
            else if (view.edit_target == APP_SIGGEN_EDIT_VPP)
            {
                app_siggen_redraw_vpp_area(&view);
                app_siggen_redraw_bottom_area(&view);
            }
            else if (view.edit_target == APP_SIGGEN_EDIT_OFFSET)
            {
                app_siggen_redraw_bottom_area(&view);
            }
            else if (view.edit_target == APP_SIGGEN_EDIT_DUTY)
            {
                app_siggen_redraw_duty_area(&view);
            }

            app_siggen_redraw_edit_area(&view);

            app_gen_start_after_ui_update(&view, config_ok);

            app_gen_wait_touch_release(300u);
            _delay_ms(30);
        }
    }
}


#define XMP_DISPLAY_RANGE       RENDER_RANGE_X1
#define XMP_TRIGGER_EDGE        TRIGGER_EDGE_RISING
#define XMP_TRIGGER_HYST_RAW    4u

static const uint16_t s_vdiv_list_mv[5] = { 250u, 500u, 1000u, 2000u, 3000u };
static const uint16_t s_tdiv_list_us[4] = { 25u, 50u, 100u, 200u };

static uint16_t pot_raw_to_y(uint8_t pot_raw) {
    uint16_t y = (uint16_t)(239u - (((uint16_t)pot_raw * 239u) / 255u));
    if (y < 15u) y = 15u;
    if (y > 220u) y = 220u;
    return y;
}

static int32_t trigger_y_to_mv(uint16_t y, uint16_t mv_per_div) {
    return (((int32_t)120 - (int32_t)y) * (int32_t)mv_per_div) / 30L;
}

static uint16_t time_div_to_samples(uint16_t time_div_us) {
    uint32_t samples = ((uint32_t)time_div_us * 10u * (uint32_t)XMP_FS_HZ) / 1000000UL;
    if (samples < 2u) samples = 2u;
    if (samples > XMP_ACQ_N) samples = XMP_ACQ_N;
    return (uint16_t)samples;
}

static void redraw_scope_frame(const volatile uint8_t *buf,
                               uint16_t start_index,
                               uint16_t trigger_y,
                               uint16_t mv_per_div,
                               uint16_t time_div_us,
                               scope_stats_t *stats,
                               uint8_t trigger_locked,
                               uint8_t force_clear)
{
    static uint16_t s_last_drawn_trigger_y = 0xFFFFu;
    char text_buf[32];
    char val_str[10];

    g_view_samples = time_div_to_samples(time_div_us);

    if (force_clear) {
        drv_ssd1289_fill_screen(LCD_COLOR_BLACK);
        drv_ssd1289_draw_grid_fast(LCD_COLOR_BLACK, LCD_COLOR_GRAY);
        svc_renderer_clear_memory();

        s_last_drawn_trigger_y = trigger_y;
        drv_ssd1289_draw_trigger_line(trigger_y, LCD_COLOR_RED);

        drv_ssd1289_draw_vdiv_label(mv_per_div);
        drv_ssd1289_draw_tdiv_label(time_div_us);
    }
    else
    {
        drv_ssd1289_update_trigger_line(trigger_y, &s_last_drawn_trigger_y, LCD_COLOR_BLACK, LCD_COLOR_GRAY, LCD_COLOR_RED);
    }

    svc_renderer_draw_waveform_vin_minmax_window(buf, XMP_ACQ_N, start_index, g_view_samples, XMP_DISPLAY_RANGE, (int32_t)mv_per_div, LCD_COLOR_YELLOW);

    if (trigger_locked) {
        drv_ssd1289_draw_text_small(5, 5, "TRIG ", LCD_COLOR_GREEN, LCD_COLOR_BLACK);
        } else {
        drv_ssd1289_draw_text_small(5, 5, "WAIT ", LCD_COLOR_RED, LCD_COLOR_BLACK);
    }

    dtostrf(stats->v_pp, 5, 3, val_str);
    sprintf(text_buf, "Vpp:%sV ", val_str);
    drv_ssd1289_draw_text_small(120, 5, text_buf, LCD_COLOR_YELLOW, LCD_COLOR_BLACK);

    if (stats->valid_freq) {
        dtostrf(stats->freq_khz, 5, 2, val_str);
        sprintf(text_buf, "f:%skHz ", val_str);
        } else {
        sprintf(text_buf, "f --- kHz ");
    }
    drv_ssd1289_draw_text_small(225, 5, text_buf, LCD_COLOR_GREEN, LCD_COLOR_BLACK);

    dtostrf(stats->v_min, 5, 3, val_str);
    sprintf(text_buf, "Vmin:%s V ", val_str);
    drv_ssd1289_draw_text_small(220, 230, text_buf, LCD_COLOR_WHITE, LCD_COLOR_BLACK);

    dtostrf(stats->v_max, 5, 3, val_str);
    sprintf(text_buf, "VmaX:%s V ", val_str);
    drv_ssd1289_draw_text_small(120, 230, text_buf, LCD_COLOR_WHITE, LCD_COLOR_BLACK);
}

static void app_run_oscilloscope_mode(void)
{
    uint8_t capture_busy = 0u;

    drv_clock_init_32mhz();
    drv_gpio_init();
    drv_ds1803_init();
    svc_hardware_offset_apply(0x69, 0xFF);
    drv_timer_ui_init_1ms();

    drv_ssd1289_init();
    drv_adc_init();
    drv_scope_controls_init();

    svc_acq_init();
    drv_timer_sample_init(XMP_FS_HZ);

    g_mv_per_div = s_vdiv_list_mv[g_vdiv_index];
    g_time_div_us = s_tdiv_list_us[g_tdiv_index];
    g_view_samples = time_div_to_samples(g_time_div_us);
    g_pretrigger_samples = g_view_samples / 10u;
    if (g_pretrigger_samples < 1u) g_pretrigger_samples = 1u;

    g_pot_raw = drv_scope_controls_read_trigger_pot_u8();
    g_trigger_line_y = pot_raw_to_y(g_pot_raw);

    svc_acq_start();
    drv_timer_sample_start();
    capture_busy = 1u;

    while (1)
    {
        drv_timer_ui_take_ticks();

        if (capture_busy && svc_acq_done())
        {
            const volatile uint8_t *buf;
            int16_t trig_idx;
            uint16_t display_start, safe_start, safe_end, posttrigger_samples;
            uint8_t draw_this_frame = 1u;

            drv_timer_sample_stop();
            capture_busy = 0u;
            buf = svc_acq_get_ch0_raw();

            scope_stats_t current_stats;
            svc_renderer_calculate_stats(buf, XMP_ACQ_N, XMP_DISPLAY_RANGE, &current_stats);

            g_pot_raw = drv_scope_controls_read_trigger_pot_u8();
            g_trigger_line_y = pot_raw_to_y(g_pot_raw);

            if (drv_scope_controls_vdiv_pressed()) {
                if (++g_vdiv_index >= 5u) g_vdiv_index = 0u;
                g_mv_per_div = s_vdiv_list_mv[g_vdiv_index];
                g_trigger_locked = 0u;
                g_force_full_redraw = 1u;
                _delay_ms(120);
            }

            if (drv_scope_controls_tdiv_pressed()) {
                if (++g_tdiv_index >= 4u) g_tdiv_index = 0u;
                g_time_div_us = s_tdiv_list_us[g_tdiv_index];
                g_trigger_locked = 0u;
                g_force_full_redraw = 1u;
                _delay_ms(120);
            }

            g_view_samples = time_div_to_samples(g_time_div_us);
            g_pretrigger_samples = g_view_samples / 10u;
            if (g_pretrigger_samples < 1u) g_pretrigger_samples = 1u;

            posttrigger_samples = (uint16_t)(g_view_samples - g_pretrigger_samples);
            safe_start = g_pretrigger_samples;
            safe_end = (posttrigger_samples >= XMP_ACQ_N) ? safe_start : (uint16_t)(XMP_ACQ_N - posttrigger_samples);
            if (safe_end <= safe_start) { safe_start = 0u; safe_end = XMP_ACQ_N; }

            g_trigger_level_mv = trigger_y_to_mv(g_trigger_line_y, g_mv_per_div);
            g_trigger_raw = svc_trigger_vin_mv_to_raw(XMP_DISPLAY_RANGE, g_trigger_level_mv);
            g_trigger_target_index = (uint16_t)(((uint32_t)safe_start + (uint32_t)safe_end) / 2u);

            trig_idx = svc_trigger_find_edge_near_u8(buf, safe_start, safe_end, g_trigger_raw, XMP_TRIGGER_EDGE, XMP_TRIGGER_HYST_RAW, g_trigger_target_index);

            if (trig_idx >= 0) {
                display_start = (uint16_t)((uint16_t)trig_idx - g_pretrigger_samples);
                if ((uint32_t)display_start + (uint32_t)g_view_samples > (uint32_t)XMP_ACQ_N) {
                    display_start = (g_view_samples >= XMP_ACQ_N) ? 0u : (uint16_t)(XMP_ACQ_N - g_view_samples);
                }
                g_last_good_display_start = display_start;
                g_trigger_locked = 1u;
                } else {
                if (g_trigger_locked) {
                    display_start = g_last_good_display_start;
                    draw_this_frame = 0u;
                    } else {
                    display_start = 0u;
                    draw_this_frame = 1u;
                }
            }

            if (draw_this_frame || g_force_full_redraw)
            {
                redraw_scope_frame(buf, display_start, g_trigger_line_y, g_mv_per_div, g_time_div_us, &current_stats, g_trigger_locked, g_force_full_redraw);
                g_force_full_redraw = 0u;
                g_frame_counter++;
            }

            svc_acq_start();
            drv_timer_sample_start();
            capture_busy = 1u;
        }

        g_loop_counter++;
    }
}

int main(void)
{
    app_mode_t selected_mode;

    /*
     * Minimal initialization for start menu and touch.
     * The oscilloscope mode below performs its own validated
     * initialization again to keep the tested oscilloscope behavior.
     */
    drv_clock_init_32mhz();
    drv_gpio_init();
    drv_timer_ui_init_1ms();

    drv_ssd1289_init();
    drv_touch_ads7843_init();

    selected_mode = app_wait_for_start_menu_selection();

    if (selected_mode == APP_MODE_OSC)
    {
        /*
         * This calls the previously validated oscilloscope code.
         * No M button and no touch handling are active inside it.
         */
        app_run_oscilloscope_mode();
    }
    else if (selected_mode == APP_MODE_FREQ)
    {
        app_run_frequency_analyzer_mode();
    }
    else if (selected_mode == APP_MODE_GEN)
    {
        app_run_gen_placeholder();
    }
    else
    {
        app_wait_for_start_menu_selection();
    }

    while (1)
    {
        /* Should never be reached. */
    }
}