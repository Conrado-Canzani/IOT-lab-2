#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "delay.h"
#include "AP_STA.h"       // inicialización WiFi AP y STA
#include "rgb_led.h"      // controla el LED RGB
#include "touchpad.h"     //  lee los 6 botones capacitivos
 
// etiqueta para los mensajes de log en el monitor serie
// aparece como prefijo: I (lab2b): ...mensaje...
static const char *TAG = "lab2b";
 

//  Archivos web embebidos en el binario

//  ESP-IDF permite incluir archivos (HTML, CSS, JS) directamente
//  dentro del firmware usando la directiva EMBED_FILES en CMakeLists.txt.
//  El compilador genera símbolos _binary_<nombre>_start y _end
//  que apuntan al inicio y fin de cada archivo en memoria flash.

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");
extern const uint8_t style_css_start[]  asm("_binary_style_css_start");
extern const uint8_t style_css_end[]    asm("_binary_style_css_end");
extern const uint8_t app_js_start[]     asm("_binary_app_js_start");
extern const uint8_t app_js_end[]       asm("_binary_app_js_end");
 

//  se separa el "color base" del "color con brillo aplicado"
//  para poder recalcular el brillo sin perder el color original.

//  led_r/g/b  - valores finales que se envían al LED (con brillo)
//  base_r/g/b  - color elegido por el usuario (sin brillo aplicado)
//  brightness - nivel de brillo, escala 0–255 (128 = mitad)

static uint8_t led_r = 0, led_g = 0, led_b = 0;
static uint8_t base_r = 255, base_g = 0, base_b = 0; // Color inicial: rojo
static uint8_t brightness = 128;                       // Brillo inicial: 50%
 
// ------------------------------------------------------------
//  apply_brightness: escala un canal de color según el brillo

//  Fórmula: resultado = color * brillo / 255
//  Se usa uint32_t en el cálculo intermedio para evitar
//  desbordamiento (overflow) antes de dividir.
//  Ejemplo: color=255, brillo=128 → 255*128/255 = 128
// ------------------------------------------------------------
static inline uint8_t apply_brightness(uint8_t color, uint8_t brightness_val)
{
    return (uint8_t)((uint32_t)color * brightness_val / 255);
}
 
 

//  cada handler es una función que se ejecuta cuando el navegador
//  hace una petición HTTP a una URL específica.
//  reciben un puntero req con la info de la petición y devuelven
//  ESP_OK si todo salió bien, o ESP_FAIL si hubo un error.

 
// GET da la página principal (index.html)
static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html"); // Le dice al navegador que es HTML
    httpd_resp_send(req, (const char *)index_html_start,
                    index_html_end - index_html_start); // Tamaño = fin - inicio
    return ESP_OK;
}
 
// GET /style.css da la hoja de estilos
static esp_err_t style_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char *)style_css_start,
                    style_css_end - style_css_start);
    return ESP_OK;
}
 
// GET /app.js da el código JavaScript de la página
static esp_err_t js_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char *)app_js_start,
                    app_js_end - app_js_start);
    return ESP_OK;
}
 
// devuelve el estado actual del LED en formato JSON
// la página web llama a este endpoint al cargar para mostrar el color activo.
// respuesta ejemplo: {"r":128,"g":0,"b":255}
static esp_err_t led_get_handler(httpd_req_t *req)
{
    char resp[64];
    snprintf(resp, sizeof(resp), "{\"r\":%d,\"g\":%d,\"b\":%d}", led_r, led_g, led_b);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN); // HTTPD_RESP_USE_STRLEN: calcula el largo automáticamente
    return ESP_OK;
}
 
// recibe un color JSON desde el navegador y aplica el cambio al LED
// cuerpo esperado: {"r":255,"g":0,"b":128}
static esp_err_t led_post_handler(httpd_req_t *req)
{
    char buf[100];

    // lee el cuerpo de la petición HTTP (el JSON que mandó el navegador)
    // MIN() evita leer más bytes de los que entran en el buffer
    int ret = httpd_req_recv(req, buf, MIN(req->content_len, sizeof(buf) - 1));
    if (ret <= 0) return ESP_FAIL; // si no llegaron datos, devuelve error

    buf[ret] = '\0'; // agrega el terminador de string para poder procesar el texto

    // intenta convertir el string recibido en un objeto JSON navegable
    cJSON *json = cJSON_Parse(buf);
    if (json == NULL) {
        // si el texto no era JSON válido, responde con error 400 al navegador
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "JSON invalido");
        return ESP_FAIL;
    }

    // busca cada campo por nombre dentro del objeto JSON
    cJSON *r_item = cJSON_GetObjectItem(json, "r");
    cJSON *g_item = cJSON_GetObjectItem(json, "g");
    cJSON *b_item = cJSON_GetObjectItem(json, "b");

    // verifica que los tres campos existan y sean números
    // si falta alguno o tiene tipo incorrecto, libera la memoria y responde error
    if (!cJSON_IsNumber(r_item) || !cJSON_IsNumber(g_item) || !cJSON_IsNumber(b_item)) {
        cJSON_Delete(json); // libera el objeto JSON antes de salir
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Faltan campos r, g o b");
        return ESP_FAIL;
    }

    // extrae los valores enteros y los guarda como color base (sin brillo aplicado)
    // se guarda el color base para poder recalcular si el brillo cambia después
    base_r = (uint8_t)r_item->valueint;
    base_g = (uint8_t)g_item->valueint;
    base_b = (uint8_t)b_item->valueint;

    cJSON_Delete(json); // libera toda la memoria del objeto JSON (siempre obligatorio)

    // aplica el brillo actual al color base para obtener los valores finales del LED
    led_r = apply_brightness(base_r, brightness);
    led_g = apply_brightness(base_g, brightness);
    led_b = apply_brightness(base_b, brightness);

    ESP_LOGI(TAG, "LED -> r:%d g:%d b:%d", led_r, led_g, led_b); // log en monitor serie
    rgb_led_set_color(led_r, led_g, led_b); // aplica el color al LED físico

    httpd_resp_send(req, NULL, 0); // responde HTTP 200 sin cuerpo
    return ESP_OK;
}
 
 
  
//  Asocia cada URL con su handler y método HTTP (GET o POST).
//  Esta estructura es requerida por esp_http_server para registrar
//  los endpoints antes de iniciar el servidor.

static const httpd_uri_t uri_root     = { .uri="/",          .method=HTTP_GET,  .handler=root_get_handler,  .user_ctx=NULL };
static const httpd_uri_t uri_css      = { .uri="/style.css",  .method=HTTP_GET,  .handler=style_get_handler, .user_ctx=NULL };
static const httpd_uri_t uri_js       = { .uri="/app.js",     .method=HTTP_GET,  .handler=js_get_handler,    .user_ctx=NULL };
static const httpd_uri_t uri_led_get  = { .uri="/led",        .method=HTTP_GET,  .handler=led_get_handler,   .user_ctx=NULL };
static const httpd_uri_t uri_led_post = { .uri="/led",        .method=HTTP_POST, .handler=led_post_handler,  .user_ctx=NULL };
 
 
// ------------------------------------------------------------
//  start_webserver: inicializa y arranca el servidor HTTP
//
//  Usa la configuración por defecto de ESP-IDF y registra
//  los 5 endpoints definidos arriba.
//  Devuelve el handle del servidor si todo salió bien, o NULL.
// ------------------------------------------------------------
static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG(); // configuración por defecto
    config.max_uri_handlers = 10;   // permite hasta 10 rutas registradas
    config.lru_purge_enable = true; // libera handlers viejos si se llena la tabla
 
    httpd_handle_t server = NULL;
    ESP_LOGI(TAG, "Iniciando servidor en puerto: %d", config.server_port); // puerto 80 por defecto
 
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registra cada URI/endpoint en el servidor
        httpd_register_uri_handler(server, &uri_root);
        httpd_register_uri_handler(server, &uri_css);
        httpd_register_uri_handler(server, &uri_js);
        httpd_register_uri_handler(server, &uri_led_get);
        httpd_register_uri_handler(server, &uri_led_post);
        return server;
    }
 
    ESP_LOGE(TAG, "Error al iniciar el servidor");
    return NULL;
}
 
 
void app_main(void)
{
    // inicializa el LED RGB (configuración de GPIO y canal PWM)
    ESP_ERROR_CHECK(rgb_led_init()); // ESP_ERROR_CHECK detiene el programa si hay error
 
    // inicializa los 6 botones capacitivos del TouchPad
    touchpad_init();
 
    // configura el ESP32 como Access Point WiFi
    // SSID: "ESP32-S2-Lab2", Contraseña: "12345678", Canal: 1, Máx conexiones: 4
    // los clientes que se conecten obtendrán IP por DHCP (ej: 192.168.4.2)
    wifi_init_ap("ESP32-S2-Lab2", "12345678", 1, 4);
 
    // inicia el servidor web HTTP en el puerto 80
    httpd_handle_t server = start_webserver();
 
    // array para detectar flanco de bajada (botón recién presionado)
    // was_pressed guarda si el botón estaba presionado en el ciclo anterior
    bool was_pressed[6] = {false};
 
    // --------------------------------------------------------
    //  loop principal: se ejecuta mientras el servidor esté activo
    //
    //  lee los 6 botones por polling cada 50ms.
    //  solo actúa cuando detecta un flanco de subida (pressed && !was_pressed),
    //  es decir, cuando el botón pasa de "no presionado" a "presionado".
    //  esto evita que la acción se repita mientras el dedo está apoyado.
    //
    //   botones:
    //   Botón 0 → Bajar brillo (mínimo 10)     (vol_up)
    //   Botón 1 → Color verde                  (play/pause)
    //   Botón 2 → Subir brillo (máximo 255)    (vol_down)
    //   Botón 3 → Color blanco                 (record)
    //   Botón 4 → Color cyan                   (photo)
    //   Botón 5 → Color magenta                (magenta)
    // --------------------------------------------------------
    while (server) {
        for (int i = 0; i < 6; i++) {
            bool pressed = touchpad_is_pressed(i);
 
            if (pressed && !was_pressed[i]) { // flanco de subida: acción nueva
                switch (i) {
                    case 0: // bajar brillo: resta 20, con piso en 10
                        brightness = (brightness > 30) ? (brightness - 20) : 10;
                        ESP_LOGI(TAG, "Brillo: %d", brightness);
                        break;
                    case 2: // subir brillo: suma 20, con techo en 255
                        brightness = (brightness < 235) ? (brightness + 20) : 255;
                        ESP_LOGI(TAG, "Brillo: %d", brightness);
                        break;
                    case 1: base_r=0;   base_g=255; base_b=0;   break; // verde
                    case 3: base_r=255; base_g=255; base_b=255; break; // blanco
                    case 4: base_r=0;   base_g=255; base_b=255; break; // cyan
                    case 5: base_r=255; base_g=0;   base_b=255; break; // magenta
                }
 
                // recalcula los valores finales aplicando el brillo actual al color base
                led_r = apply_brightness(base_r, brightness);
                led_g = apply_brightness(base_g, brightness);
                led_b = apply_brightness(base_b, brightness);
                rgb_led_set_color(led_r, led_g, led_b); // actualiza el LED físico
            }
 
            was_pressed[i] = pressed; // guarda estado para el próximo ciclo
        }
        usleep(50000); // espera 50ms antes del próximo ciclo de polling (50000 microsegundos)
    }
}