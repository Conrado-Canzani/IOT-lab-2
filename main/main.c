#include <stdio.h>
#include "driver/touch_sens.h"
#include "esp_rom_sys.h"

void app_main(void)
{
    printf("Inicio programa\n");

    touch_sensor_handle_t touch_handle = NULL;

    // 🔴 CONFIGURACIÓN BÁSICA (lo mínimo indispensable)
    touch_sensor_sample_config_t sample_cfg = {
        .clk_src = TOUCH_CLK_SRC_DEFAULT,
    };

    touch_sensor_config_t sens_cfg = {
        .sample_cfg = &sample_cfg,
    };

    esp_err_t err;

    // 🚫 SIN ESP_ERROR_CHECK (para que no crashee)
    err = touch_sensor_new_controller(&sens_cfg, &touch_handle);

    if (err != ESP_OK) {
        printf("Error creando controller: %d\n", err);
        while (1) {
            esp_rom_delay_us(1000000); // 1 segundo
        }
    }

    printf("Touch controller OK\n");

    // Loop vacío solo para ver que NO reinicia
    while (1) {
        printf("Corriendo...\n");
        esp_rom_delay_us(1000000); // 1 segundo
    }
}