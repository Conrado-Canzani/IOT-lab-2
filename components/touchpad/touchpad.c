
/*#include "touchpad.h"

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "driver/touch_sens.h"
#include "esp_check.h"
#include "esp_rom_sys.h"



#define TOUCHPAD_CHANNEL_NUM     6      // hay 6 botones táctiles
#define TOUCHPAD_INIT_SCAN_TIMES 3      // escanea 3 veces al arrancar para calibrar
#define TOUCHPAD_THRESH_RATIO    0.05f // si la señal varía 5%, se considera "tocado"

static const int s_channel_id[] = { 1, 2, 3, 5, 6, 11 }; // canales físicos del hardware



static touch_sensor_handle_t  s_sens_handle = NULL;
static touch_channel_handle_t s_chan_handle[TOUCHPAD_CHANNEL_NUM];
static uint32_t               s_threshold[TOUCHPAD_CHANNEL_NUM][TOUCH_SAMPLE_CFG_NUM];



static void touchpad_initial_scanning(void)
{
    ESP_ERROR_CHECK(touch_sensor_enable(s_sens_handle));

    for (int i = 0; i < TOUCHPAD_INIT_SCAN_TIMES; i++) {
        ESP_ERROR_CHECK(touch_sensor_trigger_oneshot_scanning(s_sens_handle, 2000));
    }

    ESP_ERROR_CHECK(touch_sensor_disable(s_sens_handle));

    printf("Benchmarks y umbrales iniciales:\n");

    for (int i = 0; i < TOUCHPAD_CHANNEL_NUM; i++) {

        uint32_t benchmark[TOUCH_SAMPLE_CFG_NUM];
        memset(benchmark, 0, sizeof(benchmark));

        
        ESP_ERROR_CHECK(touch_channel_read_data(
            s_chan_handle[i], TOUCH_CHAN_DATA_TYPE_BENCHMARK, benchmark));

        printf("[touchpad] CH %2d ->", s_channel_id[i]);

       
        touch_channel_config_t chan_cfg = {
            .active_thresh    = {0},          
            .charge_speed     = TOUCH_CHARGE_SPEED_7,
            .init_charge_volt = TOUCH_INIT_CHARGE_VOLT_DEFAULT,
        };

        for (int j = 0; j < TOUCH_SAMPLE_CFG_NUM; j++) {
            uint32_t thresh = (uint32_t)(benchmark[j] * TOUCHPAD_THRESH_RATIO);
            if (thresh == 0) thresh = 1;      
            chan_cfg.active_thresh[j] = thresh;
            s_threshold[i][j]         = thresh;
            printf("  [%d] bm=%" PRIu32 " thr=%" PRIu32, j, benchmark[j], thresh);
        }

        printf("\n");
        ESP_ERROR_CHECK(touch_sensor_reconfig_channel(s_chan_handle[i], &chan_cfg));
    }
}



void touchpad_init(void)
{
    
    touch_sensor_sample_config_t sample_cfg[TOUCH_SAMPLE_CFG_NUM] = {
        TOUCH_SENSOR_V2_DEFAULT_SAMPLE_CONFIG(
            500,                     
            TOUCH_VOLT_LIM_L_0V5,     
            TOUCH_VOLT_LIM_H_2V7      
        ),
    };

    
    touch_sensor_config_t sens_cfg =
        TOUCH_SENSOR_DEFAULT_BASIC_CONFIG(TOUCH_SAMPLE_CFG_NUM, sample_cfg);

    ESP_ERROR_CHECK(touch_sensor_new_controller(&sens_cfg, &s_sens_handle));

    
    touch_channel_config_t chan_cfg = {
        .active_thresh    = {1},
        .charge_speed     = TOUCH_CHARGE_SPEED_7,
        .init_charge_volt = TOUCH_INIT_CHARGE_VOLT_DEFAULT,
    };

    for (int i = 0; i < TOUCHPAD_CHANNEL_NUM; i++) {
        ESP_ERROR_CHECK(touch_sensor_new_channel(
            s_sens_handle, s_channel_id[i], &chan_cfg, &s_chan_handle[i]));

        touch_chan_info_t info = {};
        ESP_ERROR_CHECK(touch_sensor_get_channel_info(s_chan_handle[i], &info));
        printf("[touchpad] CH %2d habilitado en GPIO%d\n",
               s_channel_id[i], info.chan_gpio);
    }

    printf("=================================\n");

   
    touch_sensor_filter_config_t filter_cfg = TOUCH_SENSOR_DEFAULT_FILTER_CONFIG();
    ESP_ERROR_CHECK(touch_sensor_config_filter(s_sens_handle, &filter_cfg));

   
    touchpad_initial_scanning();

    
    ESP_ERROR_CHECK(touch_sensor_enable(s_sens_handle));
    ESP_ERROR_CHECK(touch_sensor_start_continuous_scanning(s_sens_handle));

    printf("[touchpad] Listo. Canales activos: ");
    for (int i = 0; i < TOUCHPAD_CHANNEL_NUM; i++) {
        printf("%d%s", s_channel_id[i],
               i < TOUCHPAD_CHANNEL_NUM - 1 ? ", " : "\n");
    }
}

bool touchpad_is_pressed(uint8_t button_index)
{
    if (button_index >= TOUCHPAD_CHANNEL_NUM) {
        return false;
    }

    uint32_t smooth[TOUCH_SAMPLE_CFG_NUM];
    memset(smooth, 0, sizeof(smooth));

    if (touch_channel_read_data(s_chan_handle[button_index],TOUCH_CHAN_DATA_TYPE_SMOOTH,smooth) != ESP_OK) {
        return false;
    }

    uint32_t benchmark[TOUCH_SAMPLE_CFG_NUM];
        memset(benchmark, 0, sizeof(benchmark));

    if (touch_channel_read_data(s_chan_handle[button_index],TOUCH_CHAN_DATA_TYPE_BENCHMARK,benchmark) != ESP_OK) {
        return false;
    }

   
    for (int j = 0; j < TOUCH_SAMPLE_CFG_NUM; j++) {
        if (smooth[j] > benchmark[j] && (smooth[j] - benchmark[j]) >= s_threshold[button_index][j]) {
            return true;
        }
    }

    return false;
}*/



#include "touchpad.h"
 
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
 
#include "driver/touch_sens.h"  // API del touchpad de ESP-IDF
#include "esp_check.h"          // para ESP_ERROR_CHECK
#include "esp_rom_sys.h"
 
 
/* ─── Configuración ──────────────────────────────────────────────── */
 
#define TOUCHPAD_CHANNEL_NUM     6      // 6 botones en la placa de expansión
#define TOUCHPAD_INIT_SCAN_TIMES 3      // cuántas veces escanear al arrancar para calibrar
#define TOUCHPAD_THRESH_RATIO    0.05f  // umbral: si la señal sube 5%, se considera tocado
                                        
 
static const int s_channel_id[] = { 1, 2, 3, 5, 6, 11 }; // canales físicos de los 6 botones
                                                        
 
 
/* ─── Estado interno ─────────────────────────────────────────────── */
 
// en el ejemplo estas eran variables locales en app_main
// las pasamos a globales estáticas para poder usarlas en touchpad_is_pressed
static touch_sensor_handle_t  s_sens_handle = NULL;
static touch_channel_handle_t s_chan_handle[TOUCHPAD_CHANNEL_NUM];
static uint32_t               s_threshold[TOUCHPAD_CHANNEL_NUM][TOUCH_SAMPLE_CFG_NUM];
// s_threshold tampoco existía en el ejemplo, lo agregamos para guardar
// los umbrales calculados y usarlos al hacer polling
 
 
/* ─── Escaneo inicial y calibración ─────────────────────────────── */
 
// esto está basado en example_touch_do_initial_scanning() del ejemplo oficial
// los cambios que hicimos:
//  - sacamos el #if SOC_TOUCH_SENSOR_VERSION porque el ESP32-S2 siempre es hw_ver2
//  - guardamos el umbral en s_threshold para poder usarlo en touchpad_is_pressed
//  - usamos TOUCHPAD_THRESH_RATIO fijo en vez del array s_thresh2bm_ratio del ejemplo
static void touchpad_initial_scanning(void)
{
    ESP_ERROR_CHECK(touch_sensor_enable(s_sens_handle));
 
    // escanea varias veces para que los valores se estabilicen
    for (int i = 0; i < TOUCHPAD_INIT_SCAN_TIMES; i++) {
        ESP_ERROR_CHECK(touch_sensor_trigger_oneshot_scanning(s_sens_handle, 2000));
    }
 
    ESP_ERROR_CHECK(touch_sensor_disable(s_sens_handle)); // hay que deshabilitar antes de reconfigurar
 
    printf("Benchmarks y umbrales iniciales:\n");
 
    for (int i = 0; i < TOUCHPAD_CHANNEL_NUM; i++) {
 
        uint32_t benchmark[TOUCH_SAMPLE_CFG_NUM];
        memset(benchmark, 0, sizeof(benchmark));
 
        // lee el benchmark = valor de la señal cuando no hay nadie tocando
        // en el ejemplo había un #if para chips más viejos, lo sacamos porque
        // el ESP32-S2 siempre soporta BENCHMARK
        ESP_ERROR_CHECK(touch_channel_read_data(
            s_chan_handle[i], TOUCH_CHAN_DATA_TYPE_BENCHMARK, benchmark));
 
        printf("[touchpad] CH %2d ->", s_channel_id[i]);
 
        // en el ejemplo usaban EXAMPLE_TOUCH_CHAN_CFG_DEFAULT()
        // lo escribimos explícito para poder asignar el umbral calculado
        touch_channel_config_t chan_cfg = {
            .active_thresh    = {0},
            .charge_speed     = TOUCH_CHARGE_SPEED_7,
            .init_charge_volt = TOUCH_INIT_CHARGE_VOLT_DEFAULT,
        };
 
        for (int j = 0; j < TOUCH_SAMPLE_CFG_NUM; j++) {
            // umbral = 5% del benchmark
            uint32_t thresh = (uint32_t)(benchmark[j] * TOUCHPAD_THRESH_RATIO);
            if (thresh == 0) thresh = 1; // mínimo 1 para que no ignore el canal
            chan_cfg.active_thresh[j] = thresh;
            s_threshold[i][j]         = thresh; // guardamos para usarlo en el polling
            printf("  [%d] bm=%" PRIu32 " thr=%" PRIu32, j, benchmark[j], thresh);
        }
 
        printf("\n");
        ESP_ERROR_CHECK(touch_sensor_reconfig_channel(s_chan_handle[i], &chan_cfg));
    }
}
 
 
/* ─── API pública ────────────────────────────────────────────────── */
 
// la secuencia de inicialización está basada en el app_main del ejemplo
// lo que cambiamos:
//  - lo metemos en una función separada para encapsular como librería
//  - sacamos los callbacks (on_active / on_inactive) porque usamos polling
//  - la config de muestras es explícita en vez de usar EXAMPLE_TOUCH_SAMPLE_CFG_DEFAULT
void touchpad_init(void)
{
    // configuración de muestras
    // usamos la macro estándar para hw_ver2 (ESP32-S2) con parámetros explícitos
    // en el ejemplo venía de EXAMPLE_TOUCH_SAMPLE_CFG_DEFAULT() que está en
    // touch_sens_example_config.h, un archivo que no queremos incluir en producción
    touch_sensor_sample_config_t sample_cfg[TOUCH_SAMPLE_CFG_NUM] = {
        TOUCH_SENSOR_V2_DEFAULT_SAMPLE_CONFIG(
            500,                       // ciclos de carga/descarga
            TOUCH_VOLT_LIM_L_0V5,      // voltaje bajo de referencia
            TOUCH_VOLT_LIM_H_2V7       // voltaje alto de referencia
        ),
    };
 
    //crear el controlador (igual que en el ejemplo)
    touch_sensor_config_t sens_cfg =
        TOUCH_SENSOR_DEFAULT_BASIC_CONFIG(TOUCH_SAMPLE_CFG_NUM, sample_cfg);
 
    ESP_ERROR_CHECK(touch_sensor_new_controller(&sens_cfg, &s_sens_handle));
 
    // registrar los 6 canales con umbral provisional = 1
    // el umbral real se calcula en touchpad_initial_scanning
    touch_channel_config_t chan_cfg = {
        .active_thresh    = {1},
        .charge_speed     = TOUCH_CHARGE_SPEED_7,
        .init_charge_volt = TOUCH_INIT_CHARGE_VOLT_DEFAULT,
    };
 
    for (int i = 0; i < TOUCHPAD_CHANNEL_NUM; i++) {
        ESP_ERROR_CHECK(touch_sensor_new_channel(
            s_sens_handle, s_channel_id[i], &chan_cfg, &s_chan_handle[i]));
 
        touch_chan_info_t info = {};
        ESP_ERROR_CHECK(touch_sensor_get_channel_info(s_chan_handle[i], &info));
        printf("[touchpad] CH %2d habilitado en GPIO%d\n",
               s_channel_id[i], info.chan_gpio);
    }
 
    printf("=================================\n");
 
    // filtro de ruido (igual que en el ejemplo)
    touch_sensor_filter_config_t filter_cfg = TOUCH_SENSOR_DEFAULT_FILTER_CONFIG();
    ESP_ERROR_CHECK(touch_sensor_config_filter(s_sens_handle, &filter_cfg));
 
    // escaneo inicial para calibrar umbrales reales (adaptado del ejemplo)
    touchpad_initial_scanning();
 
    // arrancar escaneo continuo
    // acá el ejemplo registraba callbacks antes de estas líneas
    // los sacamos porque no los necesitamos: en vez de que el hardware nos avise,
    // nosotros consultamos el estado manualmente desde main.c
    ESP_ERROR_CHECK(touch_sensor_enable(s_sens_handle));
    ESP_ERROR_CHECK(touch_sensor_start_continuous_scanning(s_sens_handle));
 
    printf("[touchpad] Listo. Canales activos: ");
    for (int i = 0; i < TOUCHPAD_CHANNEL_NUM; i++) {
        printf("%d%s", s_channel_id[i],
               i < TOUCHPAD_CHANNEL_NUM - 1 ? ", " : "\n");
    }
}
 

//   smooth    = valor actual de la señal (suavizado por el filtro)
//   benchmark = valor de referencia cuando no hay nadie tocando
//   si (smooth - benchmark) >= umbral → hay un dedo apoyado
bool touchpad_is_pressed(uint8_t button_index)
{
    if (button_index >= TOUCHPAD_CHANNEL_NUM) {
        return false; // índice inválido si es mayor a 5
    }
 
    uint32_t smooth[TOUCH_SAMPLE_CFG_NUM];
    memset(smooth, 0, sizeof(smooth));
 
    if (touch_channel_read_data(s_chan_handle[button_index],
                                TOUCH_CHAN_DATA_TYPE_SMOOTH, smooth) != ESP_OK) {
        return false;
    }
    //Lee la señal actual del botón. Se llama "smooth" porque tiene el filtro de ruido aplicado
 
    uint32_t benchmark[TOUCH_SAMPLE_CFG_NUM];
    memset(benchmark, 0, sizeof(benchmark));
 
    if (touch_channel_read_data(s_chan_handle[button_index],
                                TOUCH_CHAN_DATA_TYPE_BENCHMARK, benchmark) != ESP_OK) {
        return false;
    }
    //Lee el valor de referencia del botón, que es la señal cuando no hay nadie tocando.
 
    // si la diferencia supera el umbral calibrado al arrancar → presionado
    // basta que una muestra supere para considerarlo activo
    for (int j = 0; j < TOUCH_SAMPLE_CFG_NUM; j++) {
        if (smooth[j] > benchmark[j] &&
            (smooth[j] - benchmark[j]) >= s_threshold[button_index][j]) {
            return true;
        }
    }
    //si la señal tocando es mayor a la señál sin tocar y 
    //si la señal tocando menos la señal sin tocar es mayor o igual al 5% de esta última, 
    //el botón esta siendo tocado
 
    return false;
}