#include "AP_STA.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

static const char *TAG = "AP_STA";

static void wifi_init_common(void)
{
    static bool initialized = false;

    if (initialized)
        return;

    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    initialized = true;
}

//////////////////////////////////////////////////////////
// EVENTOS AP
//////////////////////////////////////////////////////////

static void wifi_ap_event_handler(void *arg,
                                  esp_event_base_t event_base,
                                  int32_t event_id,
                                  void *event_data)
{
    if (event_base != WIFI_EVENT)
        return;

    switch (event_id)
    {
        case WIFI_EVENT_AP_STACONNECTED://alguien se conectó a nuestra red, saca la dirección MAC
        //y lo imprime en el monitor serie.

        {
            wifi_event_ap_staconnected_t *event =
                (wifi_event_ap_staconnected_t *)event_data;

            ESP_LOGI(TAG,
                     "Cliente conectado: " MACSTR " AID=%d",
                     MAC2STR(event->mac),
                     event->aid);
            break;
        }

        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            wifi_event_ap_stadisconnected_t *event =
                (wifi_event_ap_stadisconnected_t *)event_data;

            ESP_LOGI(TAG,
                     "Cliente desconectado: " MACSTR " AID=%d",
                     MAC2STR(event->mac),
                     event->aid);
            break;
        }
            //alguien se fue, ace lo mismo
        default:
            break;
    }
}

//////////////////////////////////////////////////////////
// MODO AP
//////////////////////////////////////////////////////////

void wifi_init_ap(const char *ssid,
                  const char *password,
                  int channel,
                  int max_conn)
{
    wifi_init_common();
    // inicializa driver wifi

    esp_netif_create_default_wifi_ap();
    // crea la interfaz de red para el modo AP
    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_ap_event_handler,
            NULL));
            //// registra el handler para escuchar eventos WiFi en modo AP
    wifi_config_t ap_config = {
        .ap = {
            .channel = channel,
            .max_connection = max_conn,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        }// configura los parámetros de la red que vamos a crear
    };
    // copia SSID y contraseña a la estructura de configuración
    strcpy((char *)ap_config.ap.ssid, ssid);
    ap_config.ap.ssid_len = strlen(ssid);

    strcpy((char *)ap_config.ap.password, password);

    if (strlen(password) == 0)
    {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }// si no se puso contraseña la red queda abierta

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_AP));
        
    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_AP,
            &ap_config));

    ESP_ERROR_CHECK(
        esp_wifi_start());
    // aplica modo AP, carga la configuración y arranca el WiFi    

    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "Modo AP iniciado");
    ESP_LOGI(TAG, "SSID: %s", ssid);
    ESP_LOGI(TAG, "Password: %s", password);
    ESP_LOGI(TAG, "Canal: %d", channel);
    ESP_LOGI(TAG, "=================================");
    // confirma en el monitor que el AP está activo
}

//////////////////////////////////////////////////////////
// EVENTOS STA
//////////////////////////////////////////////////////////

static void wifi_sta_event_handler(void *arg,
                                   esp_event_base_t event_base,
                                   int32_t event_id,
                                   void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "Conectando...");
                esp_wifi_connect();
                break;// wifi arrancó: intenta conectarse al AP

            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "Conectado al AP");
                break;// conexión hecha (sin IP)

            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGW(TAG, "Desconectado. Reintentando...");
                esp_wifi_connect();
                break;// se perdió la conexión reintenta automáticamente

            default:
                break;
        }
    }

    if (event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG,
                 "IP obtenida: " IPSTR,
                 IP2STR(&event->ip_info.ip));
    }// Este evento es de IP, nos avisa que ya tenemos IP
}

//////////////////////////////////////////////////////////
// MODO STA
//////////////////////////////////////////////////////////

void wifi_init_sta(const char *ssid,
                   const char *password)
{
    wifi_init_common();
    // inicializa driver wifi

    esp_netif_create_default_wifi_sta();
    // crea la interfaz de red para el modo STA

    ESP_ERROR_CHECK(// registra el handler para eventos wifi (conexión, desconexión)
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_sta_event_handler,
            NULL));

    ESP_ERROR_CHECK(//registra también el handler para el evento de IP obtenida
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_sta_event_handler,
            NULL));

    wifi_config_t sta_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }// configura SSID y contraseña
    };

    strcpy((char *)sta_config.sta.ssid, ssid);
    strcpy((char *)sta_config.sta.password, password);

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &sta_config));

    ESP_ERROR_CHECK(
        esp_wifi_start());
    //aplica modo STA, carga la configuración y arranca el wifi

    ESP_LOGI(TAG, "Modo STA iniciado");
}