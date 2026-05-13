/*#include "touch_pad.h"
#include "led_control.h"
#include "led_strip.h"
#include "esp_bit_defs.h"
#include <unistd.h>


void app_main(void)
{
    // Inicializar el sistema de control de LED
    led_strip_t *strip;
    led_control_init(&strip);

    // Inicializar el sistema de touchpad
    touchpad_init();
    led_control_clear(strip);
    while (1) {
        if (touchpad_is_pressed()==0) {
            led_control_clear(strip);
        } else {
            led_control_set_color(strip, 0, 255, 0, 0); // Rojo
        }
    }
}*////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////LA QUE ESTABA//////////////////////////////////////////////

/*void app_main(void)
{
    led_strip_t *strip;
    led_control_init(&strip);

    touchpad_init();
    led_control_clear(strip);

    while (1) {

        uint32_t status = touchpad_is_pressed();

        if (status & BIT2) {
            // PLAY → ROJO
            led_control_set_color(strip, 0, 255, 0, 0);
        }
        else if (status & BIT5) {
            // BOTÓN 2 → VERDE
            led_control_set_color(strip, 0, 0, 255, 0);
        }
        else if (status & BIT11) {
            // BOTÓN 3 → AZUL
            led_control_set_color(strip, 0, 0, 0, 255);
        }
        else {
            led_control_clear(strip);
        }
    }
}*/
//////////////////////////////////////LA QUE ANDA PARPADEANDO//////////////////////////////

/*#include "touch_pad.h"
#include "led_control.h"
#include "led_strip.h"
#include "esp_rom_sys.h"
#include <stdio.h>
#include <unistd.h>
#include "soc/soc.h"
#include "driver/touch_pad.h"

void app_main(void)
{
    led_strip_t *strip;
    led_control_init(&strip);

    touchpad_init();

    while (1) {

        uint32_t status = touchpad_is_pressed();

        printf("STATUS: %u\n", status);

        if (status & (1 << TOUCH_PAD_NUM2)) {
            led_control_set_color(strip, 0, 255, 0, 0); // rojo
        }
        else if (status & (1 << TOUCH_PAD_NUM5)) {
            led_control_set_color(strip, 0, 0, 255, 0); // verde
        }
        else if (status & (1 << TOUCH_PAD_NUM6)) {
            led_control_set_color(strip, 0, 255, 255, 255); // blanco
        }
        else if (status & (1 << TOUCH_PAD_NUM11)) {
            led_control_set_color(strip, 0, 0, 0, 255); // azul
        }
        else {
            led_control_clear(strip);
        }

        esp_rom_delay_us(50000); // delay sin FreeRTOS (20ms)
    }
}*/
///////////////////////////funciona parpadeando pero mejor/////////////////////////////////////



/*#include "touchpad.h"
#include "rgb_led.h"
#include <unistd.h>
#include "esp_rom_sys.h"

led_strip_t *strip;

uint8_t brillo = 100;
uint8_t r = 0, g = 0, b = 0;

void actualizar_led() {
    led_control_set_color(strip, 0,
        (r * brillo) / 255,
        (g * brillo) / 255,
        (b * brillo) / 255
    );
}

void app_main(void)
{
    led_control_init(&strip);
    touchpad_init();

    while (1) {
        uint32_t status = touchpad_read();

        if (status & (1 << 0)) {
            r = 255; g = 0; b = 0;
        }
        else if (status & (1 << 1)) {
            r = 0; g = 255; b = 0;
        }
        else if (status & (1 << 2)) {
            r = 0; g = 0; b = 255;
        }
        else if (status & (1 << 3)) {
            brillo += 10;
        }
        else if (status & (1 << 4)) {
            brillo -= 10;
        }

        actualizar_led();
        esp_rom_delay_us(50000);
    }
}*/
///////////////////////////////////////////////////////////////////////
/////////////////////este anda para prender el led de varios colores////////////////////////

/*#include "esp_log.h"
#include "rgb_led.h"
#include "sdkconfig.h"
#include "esp_rom_sys.h"
#include "delay.h"

static const char *TAG = "main";

void app_main(void)
{
    rgb_led_init();

    while (1) {

        ESP_LOGI(TAG, "RED");
        rgb_led_red();
        //esp_rom_delay_us(1000000); // 1 segundo
        delay_s(1);

        ESP_LOGI(TAG, "GREEN");
        rgb_led_green();
        //esp_rom_delay_us(1000000); // 1 segundo
        delay_s(1);

        ESP_LOGI(TAG, "BLUE");
        rgb_led_blue();
        //esp_rom_delay_us(1000000); // 1 segundo
        delay_s(1);

        ESP_LOGI(TAG, "WHITE");
        rgb_led_white();
        //esp_rom_delay_us(1000000); // 1 segundo
        delay_s(1);

        ESP_LOGI(TAG, "CLEAR");
        rgb_led_clear();
        //esp_rom_delay_us(1000000); // 1 segundo
        delay_s(1);
    }
}*/

/*#include <stdio.h>
#include <inttypes.h>

#include "driver/touch_sens.h"
#include "esp_check.h"
#include "esp_rom_sys.h"

//#include "touch_sens_example_config.h"
#include "rgb_led.h"

// ======================================================
// CONFIGURACIÓN
// ======================================================

//#define EXAMPLE_TOUCH_SAMPLE_CFG_NUM         TOUCH_SAMPLE_CFG_NUM
#define EXAMPLE_TOUCH_SAMPLE_CFG_NUM 1
#define EXAMPLE_TOUCH_CHANNEL_NUM            6
#define EXAMPLE_TOUCH_CHAN_INIT_SCAN_TIMES  3

#define EXAMPLE_TOUCH_SAMPLE_CFG_NUM TOUCH_SAMPLE_CFG_NUM

#define EXAMPLE_TOUCH_CHAN_CFG_DEFAULT() \
    TOUCH_CHANNEL_DEFAULT_CONFIG()

// Sensibilidad
static float s_thresh2bm_ratio[EXAMPLE_TOUCH_CHANNEL_NUM] = {
    [0 ... EXAMPLE_TOUCH_CHANNEL_NUM - 1] = 0.015f,
};

// Canales touch
static int s_channel_id[EXAMPLE_TOUCH_CHANNEL_NUM] = {
    1,
    2,
    3,
    5,
    6,
    11,
};

// ======================================================
// VARIABLES GLOBALES
// ======================================================

static uint8_t brillo = 255;

// Color actual
static uint8_t current_r = 255;
static uint8_t current_g = 255;
static uint8_t current_b = 255;

// ======================================================
// ACTUALIZAR LED
// ======================================================

static void actualizar_led(void)
{
    rgb_led_set_color(
        (current_r * brillo) / 255,
        (current_g * brillo) / 255,
        (current_b * brillo) / 255
    );
}

// ======================================================
// CALLBACK TOUCH ACTIVO
// ======================================================

bool example_touch_on_active_callback(
    touch_sensor_handle_t sens_handle,
    const touch_active_event_data_t *event,
    void *user_ctx
)
{
    int ch = event->chan_id;

    printf("Touch CH%d\n", ch);

    switch (ch) {

        // =========================
        // BAJAR BRILLO
        // =========================
        case 1:

            if (brillo >= 25) {
                brillo -= 25;
            }

            printf("Brillo: %d\n", brillo);

            actualizar_led();

            break;

        // =========================
        // ROJO
        // =========================
        case 2:

            current_r = 255;
            current_g = 0;
            current_b = 0;

            actualizar_led();

            break;

        // =========================
        // SUBIR BRILLO
        // =========================
        case 3:

            if (brillo <= 230) {
                brillo += 25;
            }

            printf("Brillo: %d\n", brillo);

            actualizar_led();

            break;

        // =========================
        // VERDE
        // =========================
        case 5:

            current_r = 0;
            current_g = 255;
            current_b = 0;

            actualizar_led();

            break;

        // =========================
        // BLANCO
        // =========================
        case 6:

            current_r = 255;
            current_g = 255;
            current_b = 255;

            actualizar_led();

            break;

        // =========================
        // AZUL
        // =========================
        case 11:

            current_r = 0;
            current_g = 0;
            current_b = 255;

            actualizar_led();

            break;

        default:
            break;
    }

    return false;
}

// ======================================================
// CALLBACK TOUCH INACTIVO
// ======================================================

bool example_touch_on_inactive_callback(
    touch_sensor_handle_t sens_handle,
    const touch_inactive_event_data_t *event,
    void *user_ctx
)
{
    return false;
}

// ======================================================
// ESCANEO INICIAL
// ======================================================

static void example_touch_do_initial_scanning(
    touch_sensor_handle_t sens_handle,
    touch_channel_handle_t chan_handle[]
)
{
    ESP_ERROR_CHECK(touch_sensor_enable(sens_handle));

    for (int i = 0; i < EXAMPLE_TOUCH_CHAN_INIT_SCAN_TIMES; i++) {

        ESP_ERROR_CHECK(
            touch_sensor_trigger_oneshot_scanning(
                sens_handle,
                2000
            )
        );
    }

    ESP_ERROR_CHECK(touch_sensor_disable(sens_handle));

    printf("Initial benchmark and thresholds:\n");

    for (int i = 0; i < EXAMPLE_TOUCH_CHANNEL_NUM; i++) {

        uint32_t benchmark[EXAMPLE_TOUCH_SAMPLE_CFG_NUM] = {};

#if SOC_TOUCH_SUPPORT_BENCHMARK
        ESP_ERROR_CHECK(
            touch_channel_read_data(
                chan_handle[i],
                TOUCH_CHAN_DATA_TYPE_BENCHMARK,
                benchmark
            )
        );
#else
        ESP_ERROR_CHECK(
            touch_channel_read_data(
                chan_handle[i],
                TOUCH_CHAN_DATA_TYPE_SMOOTH,
                benchmark
            )
        );
#endif

        printf("Touch [CH %d]\n", s_channel_id[i]);

        touch_channel_config_t chan_cfg =
            EXAMPLE_TOUCH_CHAN_CFG_DEFAULT();

        for (int j = 0; j < EXAMPLE_TOUCH_SAMPLE_CFG_NUM; j++) {

#if SOC_TOUCH_SENSOR_VERSION == 1

            chan_cfg.abs_active_thresh[j] =
                (uint32_t)(
                    benchmark[j] *
                    (1 - s_thresh2bm_ratio[i])
                );

#else

            chan_cfg.active_thresh[j] =
                (uint32_t)(
                    benchmark[j] *
                    s_thresh2bm_ratio[i]
                );

#endif
        }

        ESP_ERROR_CHECK(
            touch_sensor_reconfig_channel(
                chan_handle[i],
                &chan_cfg
            )
        );
    }
}

// ======================================================
// APP MAIN
// ======================================================

void app_main(void)
{
    // Inicializar RGB
    rgb_led_init();

    // Inicializar Touch
    touch_sensor_handle_t sens_handle = NULL;

    touch_channel_handle_t
        chan_handle[EXAMPLE_TOUCH_CHANNEL_NUM];

    touch_sensor_sample_config_t sample_cfg[
        TOUCH_SAMPLE_CFG_NUM
    ] = EXAMPLE_TOUCH_SAMPLE_CFG_DEFAULT();
    //touch_sensor_sample_config_t sample_cfg[1] = {
    //{
    //    .sample_num = 1,
    //    .interval_ms = 10,
    //}
//};

    touch_sensor_config_t sens_cfg =
        TOUCH_SENSOR_DEFAULT_BASIC_CONFIG(
            EXAMPLE_TOUCH_SAMPLE_CFG_NUM,
            sample_cfg
        );

    ESP_ERROR_CHECK(
        touch_sensor_new_controller(
            &sens_cfg,
            &sens_handle
        )
    );

    touch_channel_config_t chan_cfg =
       EXAMPLE_TOUCH_CHAN_CFG_DEFAULT();
    //touch_channel_config_t chan_cfg = {
    //.active_hysteresis = 1,
    //.debounce_cnt = 1,
    //.noise_thr = 0,
    //.jitter_step = 4,
    //.smh_lvl = TOUCH_PAD_SMOOTH_LEVEL2,
//};

    for (int i = 0; i < EXAMPLE_TOUCH_CHANNEL_NUM; i++) {

        ESP_ERROR_CHECK(
            touch_sensor_new_channel(
                sens_handle,
                s_channel_id[i],
                &chan_cfg,
                &chan_handle[i]
            )
        );

        touch_chan_info_t chan_info = {};

        ESP_ERROR_CHECK(
            touch_sensor_get_channel_info(
                chan_handle[i],
                &chan_info
            )
        );

        printf(
            "Touch [CH %d] enabled on GPIO%d\n",
            s_channel_id[i],
            chan_info.chan_gpio
        );
    }

    touch_sensor_filter_config_t filter_cfg =
        TOUCH_SENSOR_DEFAULT_FILTER_CONFIG();

    ESP_ERROR_CHECK(
        touch_sensor_config_filter(
            sens_handle,
            &filter_cfg
        )
    );

    // Calibración inicial
    example_touch_do_initial_scanning(
        sens_handle,
        chan_handle
    );

    // Registrar callbacks
    touch_event_callbacks_t callbacks = {
        .on_active = example_touch_on_active_callback,
        .on_inactive = example_touch_on_inactive_callback,
    };

    ESP_ERROR_CHECK(
        touch_sensor_register_callbacks(
            sens_handle,
            &callbacks,
            NULL
        )
    );

    // Habilitar touch
    ESP_ERROR_CHECK(
        touch_sensor_enable(sens_handle)
    );

    ESP_ERROR_CHECK(
        touch_sensor_start_continuous_scanning(
            sens_handle
        )
    );

    // Color inicial
    actualizar_led();

    // Loop vacío
    while (1) {
        esp_rom_delay_us(100000);
    }
}*/
/*#include <stdio.h>
#include <inttypes.h>

#include "driver/touch_sens.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_task_wdt.h"

#include "rgb_led.h"

// ===========================
// CONFIGURACIÓN TOUCH
// ===========================

#define TOUCH_CHANNEL_NUM 1

static int s_channel_id[TOUCH_CHANNEL_NUM] = {
    2   // Canal touch 2
};

// ===========================
// CALLBACKS
// ===========================

bool touch_active_callback(
    touch_sensor_handle_t sens_handle,
    const touch_active_event_data_t *event,
    void *user_ctx
)
{
    ESP_LOGI("TOUCH", "TOUCH ACTIVO CH %d", event->chan_id);

    // LED ROJO
    //rgb_led_set_color(255, 0, 0);
    rgb_led_red();

    return false;
}

bool touch_inactive_callback(
    touch_sensor_handle_t sens_handle,
    const touch_inactive_event_data_t *event,
    void *user_ctx
)
{
    ESP_LOGI("TOUCH", "TOUCH INACTIVO CH %d", event->chan_id);

    // LED APAGADO
    //rgb_led_set_color(0, 0, 0);
    rgb_led_clear();

    return false;
}

// ===========================
// MAIN
// ===========================

void app_main(void)
{
    esp_task_wdt_deinit();
    //esp_task_wdt_delete(NULL);
    // ===========================
    // INICIALIZAR LED RGB
    // ===========================

    rgb_led_init();

    //rgb_led_set_color(0, 0, 0);
    rgb_led_clear();

    // ===========================
    // TOUCH SENSOR
    // ===========================

    touch_sensor_handle_t sens_handle = NULL;

    touch_channel_handle_t chan_handle[TOUCH_CHANNEL_NUM];

    // Configuración básica
    //touch_sensor_config_t sens_cfg =
        //TOUCH_SENSOR_DEFAULT_BASIC_CONFIG(0, NULL);

    //ESP_ERROR_CHECK(
        //touch_sensor_new_controller(
            //&sens_cfg,
            //&sens_handle
        //)
    //);
    // Configuración sample obligatoria en IDF 6

//touch_sensor_sample_config_t sample_cfg = {
    //.sample_freq_hz = 100,
//};

//touch_sensor_sample_config_t sample_cfg =
    //TOUCH_SENSOR_V2_DEFAULT_SAMPLE_CONFIG();
touch_sensor_sample_config_t sample_cfg =
    TOUCH_SENSOR_V2_DEFAULT_SAMPLE_CONFIG(
        TOUCH_CHARGE_SPEED_4,
        TOUCH_VOLT_LIM_L_0V5,
        TOUCH_VOLT_LIM_H_2V7
    );

touch_sensor_config_t sens_cfg = {
    .sample_cfg = &sample_cfg,
    .sample_cfg_num = 1,
};

//touch_sensor_config_t sens_cfg = {
//    .sample_cfg = &sample_cfg,
//};

ESP_ERROR_CHECK(
    touch_sensor_new_controller(
        &sens_cfg,
        &sens_handle
    )
);

    // Configuración canal
    touch_channel_config_t chan_cfg = {
        .active_thresh = { 3000 }
    };

    // Crear canal
    for (int i = 0; i < TOUCH_CHANNEL_NUM; i++) {

        ESP_ERROR_CHECK(
            touch_sensor_new_channel(
                sens_handle,
                s_channel_id[i],
                &chan_cfg,
                &chan_handle[i]
            )
        );

        touch_chan_info_t chan_info = {};

        ESP_ERROR_CHECK(
            touch_sensor_get_channel_info(
                chan_handle[i],
                &chan_info
            )
        );

        printf(
            "Touch [CH %d] en GPIO%d\n",
            s_channel_id[i],
            chan_info.chan_gpio
        );
    }

    // ===========================
    // CALLBACKS
    // ===========================

    touch_event_callbacks_t callbacks = {
        .on_active = touch_active_callback,
        .on_inactive = touch_inactive_callback,
    };

    ESP_ERROR_CHECK(
        touch_sensor_register_callbacks(
            sens_handle,
            &callbacks,
            NULL
        )
    );

    // ===========================
    // INICIAR TOUCH
    // ===========================

    ESP_ERROR_CHECK(
        touch_sensor_enable(sens_handle)
    );

    ESP_ERROR_CHECK(
        touch_sensor_start_continuous_scanning(
            sens_handle
        )
    );

    printf("Touch iniciado\n");

    // ===========================
    // LOOP SIN FREERTOS
    // ===========================

    while (1)
{
    leer_touch();

    esp_rom_delay_us(100);

   }   taskYIELD();
}*/

/*#include "touchpad.h"
#include "rgb_led.h"

led_strip_t *strip;

uint8_t brillo = 100;
uint8_t r = 0, g = 0, b = 0;

void actualizar_led()
{
    led_control_set_color(strip, 0,
        (r * brillo) / 255,
        (g * brillo) / 255,
        (b * brillo) / 255
    );
}

void app_main(void)
{
    led_control_init(&strip);
    touchpad_init();

    while (1)
    {
        uint32_t status = touchpad_read();

        if (status & (1 << 0)) { r = 255; g = 0; b = 0; }
        else if (status & (1 << 1)) { r = 0; g = 255; b = 0; }
        else if (status & (1 << 2)) { r = 0; g = 0; b = 255; }
        else if (status & (1 << 3)) { if (brillo <= 245) brillo += 10; }
        else if (status & (1 << 4)) { if (brillo >= 10) brillo -= 10; }

        actualizar_led();

        esp_rom_delay_us(100);
    }
}*/
/*#include "rgb_led.h"
#include "touchpad.h"
#include "esp_rom_sys.h"

void app_main(void)
{
    rgb_led_init();
    touchpad_init();

    while (1)
    {
        if (touchpad_is_pressed(0)) {
            rgb_led_red();
        }
        else if (touchpad_is_pressed(1)) {
            rgb_led_green();
        }
        else if (touchpad_is_pressed(2)) {
            rgb_led_blue();
        }
        else {
            rgb_led_clear();
        }

        esp_rom_delay_us(10000); // 🔴 importante
    }
}*/////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include "esp_rom_sys.h"

#include "touchpad.h"
#include "rgb_led.h"

/*
 * Mapeo de botones:
 *
 *  Índice │ Canal │ Función
 * ────────┼───────┼──────────────────────────
 *    0    │   1   │ Bajar brillo
 *    1    │   2   │ Color Verde
 *    2    │   3   │ Subir brillo
 *    3    │   5   │ Color Blanco
 *    4    │   6   │ Color Cian
 *    5    │  11   │ Color Magenta
 */

#define TOUCHPAD_NUM_BUTTONS    6
#define POLL_INTERVAL_US        1000   /* 50 ms entre lecturas              */

#define BRIGHTNESS_STEP         20      /* cuánto sube/baja por pulsación    */
#define BRIGHTNESS_MIN          10      /* mínimo para que se vea algo       */
#define BRIGHTNESS_MAX          255

static inline uint8_t apply_brightness(uint8_t color, uint8_t brightness)
{
    return (uint8_t)((uint32_t)color * brightness / 255);
}

void app_main(void)
{
    rgb_led_init();
    touchpad_init();

    printf("[main] Sistema listo. Brillo inicial: 50%%\n");

    uint8_t brightness = 128;   /* 50% inicial */
    uint8_t base_r     = 255;   /* color base inicial: Rojo */
    uint8_t base_g     = 0;
    uint8_t base_b     = 0;

    /* Detección de flanco: solo actúa al presionar, no mientras se mantiene */
    bool was_pressed[TOUCHPAD_NUM_BUTTONS] = {false};

    while (1) {

        for (uint8_t i = 0; i < TOUCHPAD_NUM_BUTTONS; i++) {

            bool pressed = touchpad_is_pressed(i);

            if (pressed && !was_pressed[i]) {   /* flanco de subida */

                switch (i) {

                    case 0:  /* Canal 1 → Bajar brillo */
                        brightness = (brightness > BRIGHTNESS_MIN + BRIGHTNESS_STEP)
                                     ? (brightness - BRIGHTNESS_STEP)
                                     : BRIGHTNESS_MIN;
                        printf("[main] Brillo bajo: %d/255\n", brightness);
                        break;

                    case 2:  /* Canal 3 → Subir brillo */
                        brightness = (brightness < BRIGHTNESS_MAX - BRIGHTNESS_STEP)
                                     ? (brightness + BRIGHTNESS_STEP)
                                     : BRIGHTNESS_MAX;
                        printf("[main] Brillo alto: %d/255\n", brightness);
                        break;

                    case 1:  /* Canal 2 → Verde */
                        base_r = 0; base_g = 255; base_b = 0;
                        printf("[main] Color: Verde\n");
                        break;

                    case 3:  /* Canal 5 → Blanco */
                        base_r = 255; base_g = 255; base_b = 255;
                        printf("[main] Color: Blanco\n");
                        break;

                    case 4:  /* Canal 6 → Cian */
                        base_r = 0; base_g = 255; base_b = 255;
                        printf("[main] Color: Cian\n");
                        break;

                    case 5:  /* Canal 11 → Magenta */
                        base_r = 255; base_g = 0; base_b = 255;
                        printf("[main] Color: Magenta\n");
                        break;

                    default:
                        break;
                }
            }

            was_pressed[i] = pressed;
        }

        /* Aplicar brillo al color base y actualizar el LED */
        rgb_led_set_color(
            apply_brightness(base_r, brightness),
            apply_brightness(base_g, brightness),
            apply_brightness(base_b, brightness)
        );

        esp_rom_delay_us(POLL_INTERVAL_US);
    }
}