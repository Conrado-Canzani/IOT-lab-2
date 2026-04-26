#include "touchpad.h"
#include "driver/touch_pad.h"
#include "esp_log.h"
#include "esp_rom_sys.h"  // 👈 IMPORTANTE

#define TAG "TOUCH"

static const touch_pad_t touch_pins[TOUCHPAD_NUM_BUTTONS] = {
    TOUCH_PAD_NUM0,
    TOUCH_PAD_NUM1,
    TOUCH_PAD_NUM2,
    TOUCH_PAD_NUM3,
    TOUCH_PAD_NUM4,
    TOUCH_PAD_NUM5
};

static uint32_t threshold[TOUCHPAD_NUM_BUTTONS];

static bool current_state[TOUCHPAD_NUM_BUTTONS];
static bool previous_state[TOUCHPAD_NUM_BUTTONS];

void touchpad_init(void)
{
    touch_pad_init();

    for (int i = 0; i < TOUCHPAD_NUM_BUTTONS; i++) {
        touch_pad_config(touch_pins[i], 0);
    }

    // 👇 Delay sin FreeRTOS
    esp_rom_delay_us(100000); // 100 ms

    for (int i = 0; i < TOUCHPAD_NUM_BUTTONS; i++) {
        uint32_t val;
        touch_pad_read_raw_data(touch_pins[i], &val);

        threshold[i] = val * 0.8;

        ESP_LOGI(TAG, "Pad %d base=%lu threshold=%lu", i, val, threshold[i]);

        current_state[i] = false;
        previous_state[i] = false;
    }
}