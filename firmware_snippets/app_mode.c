/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   app_mode.c                               */
/*    Zweck:   Mode management for 3-in-1 device        */
/********************************************************/

#include "app_mode.h"

static app_mode_t s_app_mode = APP_MODE_MENU;

void app_mode_init(void)
{
    s_app_mode = APP_MODE_MENU;
}

void app_mode_set(app_mode_t mode)
{
    if ((mode == APP_MODE_MENU) ||
        (mode == APP_MODE_OSC)  ||
        (mode == APP_MODE_FREQ) ||
        (mode == APP_MODE_GEN))
    {
        s_app_mode = mode;
    }
    else
    {
        s_app_mode = APP_MODE_MENU;
    }
}

app_mode_t app_mode_get(void)
{
    return s_app_mode;
}

void app_mode_next(void)
{
    if (s_app_mode == APP_MODE_MENU)
    {
        s_app_mode = APP_MODE_OSC;
    }
    else if (s_app_mode == APP_MODE_OSC)
    {
        s_app_mode = APP_MODE_FREQ;
    }
    else if (s_app_mode == APP_MODE_FREQ)
    {
        s_app_mode = APP_MODE_GEN;
    }
    else
    {
        s_app_mode = APP_MODE_MENU;
    }
}