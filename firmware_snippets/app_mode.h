/********************************************************/
/*    Projekt: XMEGA-Messplattform (XMP)                */
/*    Datei:   app_mode.h                               */
/*    Zweck:   Mode management for 3-in-1 device        */
/********************************************************/
#pragma once

#include <stdint.h>

typedef enum
{
    APP_MODE_MENU = 0u,
    APP_MODE_OSC  = 1u,
    APP_MODE_FREQ = 2u,
    APP_MODE_GEN  = 3u

} app_mode_t;

void app_mode_init(void);
void app_mode_set(app_mode_t mode);
app_mode_t app_mode_get(void);
void app_mode_next(void);