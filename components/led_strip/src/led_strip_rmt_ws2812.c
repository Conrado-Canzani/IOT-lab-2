#include "led_strip.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

#define TAG "LED_STRIP"

// WS2812 timing (en nanosegundos)
#define T0H 350
#define T0L 1000
#define T1H 1000
#define T1L 350

#define RMT_RESOLUTION_HZ 10000000 // 10 MHz

typedef struct {
    led_strip_t parent;
    rmt_channel_handle_t channel;
    rmt_encoder_handle_t encoder;
    uint32_t length;
    uint8_t *buffer;
} ws2812_strip_t;

// ==========================
// ENCODING
// ==========================

static void ws2812_send(ws2812_strip_t *strip)
{
    int total_bits = strip->length * 24;
    rmt_symbol_word_t *symbols = calloc(total_bits, sizeof(rmt_symbol_word_t));

    if (!symbols) return;

    float ratio = (float)RMT_RESOLUTION_HZ / 1e9;

    int idx = 0;

    for (int i = 0; i < strip->length * 3; i++) {
        uint8_t byte = strip->buffer[i];

        for (int bit = 7; bit >= 0; bit--) {

            if (byte & (1 << bit)) {
                symbols[idx] = (rmt_symbol_word_t){
                    .level0 = 1,
                    .duration0 = (uint32_t)(T1H * ratio),
                    .level1 = 0,
                    .duration1 = (uint32_t)(T1L * ratio),
                };
            } else {
                symbols[idx] = (rmt_symbol_word_t){
                    .level0 = 1,
                    .duration0 = (uint32_t)(T0H * ratio),
                    .level1 = 0,
                    .duration1 = (uint32_t)(T0L * ratio),
                };
            }

            idx++;
        }
    }

    rmt_transmit_config_t tx_config = {
        .loop_count = 0
    };

    // 🔥 FIX: usar encoder
    rmt_transmit(strip->channel, strip->encoder, symbols,
                 total_bits * sizeof(rmt_symbol_word_t), &tx_config);

    // 🔥 FIX: esperar correctamente
    rmt_tx_wait_all_done(strip->channel, -1);

    free(symbols);
}

// ==========================
// API
// ==========================

static esp_err_t ws2812_set_pixel(led_strip_t *strip, uint32_t index,
                                  uint8_t red, uint8_t green, uint8_t blue)
{
    ws2812_strip_t *ws = (ws2812_strip_t *)strip;

    if (index >= ws->length) {
        return ESP_ERR_INVALID_ARG;
    }

    uint32_t pos = index * 3;

    // GRB (WS2812)
    ws->buffer[pos + 0] = green;
    ws->buffer[pos + 1] = red;
    ws->buffer[pos + 2] = blue;

    return ESP_OK;
}

static esp_err_t ws2812_refresh(led_strip_t *strip)
{
    ws2812_strip_t *ws = (ws2812_strip_t *)strip;
    ws2812_send(ws);
    return ESP_OK;
}

static esp_err_t ws2812_clear(led_strip_t *strip)
{
    ws2812_strip_t *ws = (ws2812_strip_t *)strip;
    memset(ws->buffer, 0, ws->length * 3);
    ws2812_send(ws);
    return ESP_OK;
}

static esp_err_t ws2812_del(led_strip_t *strip)
{
    ws2812_strip_t *ws = (ws2812_strip_t *)strip;

    if (ws->encoder) {
        rmt_del_encoder(ws->encoder);
    }

    if (ws->channel) {
        rmt_disable(ws->channel);
        rmt_del_channel(ws->channel);
    }

    if (ws->buffer) free(ws->buffer);
    free(ws);

    return ESP_OK;
}

// ==========================
// INIT
// ==========================

led_strip_t *led_strip_new_ws2812(uint32_t gpio, uint32_t led_count)
{
    ws2812_strip_t *ws = calloc(1, sizeof(ws2812_strip_t));
    if (!ws) return NULL;

    ws->length = led_count;

    ws->buffer = calloc(led_count * 3, sizeof(uint8_t));
    if (!ws->buffer) {
        free(ws);
        return NULL;
    }

    // Config RMT TX
    rmt_tx_channel_config_t config = {
        .gpio_num = gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RMT_RESOLUTION_HZ,
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };

    if (rmt_new_tx_channel(&config, &ws->channel) != ESP_OK) {
        free(ws->buffer);
        free(ws);
        return NULL;
    }

    // 🔥 Crear encoder (clave en IDF 6.0)
    rmt_copy_encoder_config_t encoder_config = {};
    if (rmt_new_copy_encoder(&encoder_config, &ws->encoder) != ESP_OK) {
        rmt_del_channel(ws->channel);
        free(ws->buffer);
        free(ws);
        return NULL;
    }

    rmt_enable(ws->channel);

    // Asignar funciones
    ws->parent.set_pixel = ws2812_set_pixel;
    ws->parent.refresh   = ws2812_refresh;
    ws->parent.clear     = ws2812_clear;
    ws->parent.del       = ws2812_del;

    return &ws->parent;
}