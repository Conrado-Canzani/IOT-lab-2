#pragma once

#include <stdint.h>
#include "esp_err.h"

// Estructura tipo "driver"
typedef struct led_strip_t led_strip_t;

struct led_strip_t {
    esp_err_t (*set_pixel)(led_strip_t *strip, uint32_t index, uint8_t red, uint8_t green, uint8_t blue);
    esp_err_t (*refresh)(led_strip_t *strip);
    esp_err_t (*clear)(led_strip_t *strip);
    esp_err_t (*del)(led_strip_t *strip);
};

// Crear nuevo strip (WS2812 con RMT)
led_strip_t *led_strip_new_ws2812(uint32_t gpio, uint32_t led_count);