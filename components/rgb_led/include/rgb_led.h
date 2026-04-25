#pragma once

#include "esp_err.h"

// Inicializa el LED
esp_err_t rgb_led_init(void);

// Funciones de colores
void rgb_led_set_red(void);
void rgb_led_set_green(void);
void rgb_led_set_blue(void);
void rgb_led_off(void);