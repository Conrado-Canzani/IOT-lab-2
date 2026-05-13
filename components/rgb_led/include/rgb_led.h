/*#pragma once
#include "led_strip.h"

extern led_strip_handle_t strip;
// Inicializa el LED
esp_err_t rgb_led_init(void);

// Funciones de colores
void rgb_led_set_red(void);
void rgb_led_set_green(void);
void rgb_led_set_blue(void);
void rgb_led_off(void);*/

#ifndef RGB_LED_H
#define RGB_LED_H

#include "esp_err.h"

esp_err_t rgb_led_init(void);

void rgb_led_red(void);
void rgb_led_green(void);
void rgb_led_blue(void);
void rgb_led_white(void);
void rgb_led_clear(void);
void rgb_led_set_color(uint8_t r, uint8_t g, uint8_t b);

#endif