#include "rgb_led.h"
#include "led_strip.h"

static led_strip_t *strip = NULL;

// Inicialización
esp_err_t rgb_led_init(void)
{
    // GPIO 45 -> cambiá si usás otro pin
    strip = led_strip_new_ws2812(45, 1);

    if (strip == NULL) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

// Funciones de colores
void rgb_led_set_red(void)
{
    strip->set_pixel(strip, 0, 255, 0, 0);
    strip->refresh(strip);
}

void rgb_led_set_green(void)
{
    strip->set_pixel(strip, 0, 0, 255, 0);
    strip->refresh(strip);
}

void rgb_led_set_blue(void)
{
    strip->set_pixel(strip, 0, 0, 0, 255);
    strip->refresh(strip);
}

void rgb_led_off(void)
{
    strip->clear(strip);
}