//#include "led_strip.h"
//#include "rgb_led.h"

//led_strip_t *strip = NULL;
//static led_strip_t *strip = NULL;

// Inicialización
/*esp_err_t rgb_led_init(void)
{
    // GPIO 45 -> cambiá si usás otro pin
    strip = led_strip_new_ws2812(45, 1);

    if (strip == NULL) {
        return ESP_FAIL;
    }

    return ESP_OK;
}*/
/*#include "led_strip.h"
#include "driver/rmt_tx.h"

led_strip_handle_t strip = NULL;

void rgb_led_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = 45,   // tu pin
        .max_leds = 1,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &strip));

    led_strip_clear(strip);
}

// Funciones de colores
void rgb_led_set_red(void)
{
    //strip->set_pixel(strip, 0, 255, 0, 0);
    //strip->refresh(strip);
    led_strip_set_pixel(strip, 0, 255, 0, 0); // rojo
    led_strip_refresh(strip);
}

void rgb_led_set_green(void)
{
    //strip->set_pixel(strip, 0, 0, 255, 0);
    //strip->refresh(strip);
    led_strip_set_pixel(strip, 0, 0, 255, 0); // rojo
    led_strip_refresh(strip);
}

void rgb_led_set_blue(void)
{
    //strip->set_pixel(strip, 0, 0, 0, 255);
    //strip->refresh(strip);
    led_strip_set_pixel(strip, 0, 0, 0, 255); // rojo
    led_strip_refresh(strip);
}

void rgb_led_off(void)
{
    strip->clear(strip);
}*/
#define LED_STRIP_RMT_BACKEND
#include "rgb_led.h"
#include "led_strip.h"
#include "esp_log.h"
#include "sdkconfig.h"

static const char *TAG = "rgb_led";
#define LED_GPIO 45
//#define LED_GPIO CONFIG_BLINK_GPIO

static led_strip_handle_t strip;

/*esp_err_t rgb_led_init(void)
{
    ESP_LOGI(TAG, "Initializing RGB LED (WS2812)");

    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1,
    };

#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &strip));

#else
#error "Only RMT backend supported in this setup"
#endif

    led_strip_clear(strip);
    return ESP_OK;
}*/esp_err_t rgb_led_init(void)
{
    ESP_LOGI(TAG, "Initializing RGB LED (WS2812)");

    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &strip
        )
    );

    led_strip_clear(strip);

    return ESP_OK;
}

static void set_color(uint8_t r, uint8_t g, uint8_t b)
{
    led_strip_set_pixel(strip, 0, r, g, b);
    led_strip_refresh(strip);
}

void rgb_led_red(void)
{
    set_color(255, 0, 0);
}

void rgb_led_green(void)
{
    set_color(0, 255, 0);
}

void rgb_led_blue(void)
{
    set_color(0, 0, 255);
}

void rgb_led_white(void)
{
    set_color(255, 255, 255);
}

void rgb_led_clear(void)
{
    led_strip_clear(strip);
}

void rgb_led_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    set_color(r, g, b);
}