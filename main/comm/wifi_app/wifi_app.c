//
// Created by Pratham Jaiswal on 20/03/24.
//

#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "lwip/netdb.h"

#include "wifi_app.h"
#include "settings.h"
#include "tasks_common.h"
#include "mdns.h"
#include "comm_if.h"
#include "silion_sim7200.h"
#include "app_state_disp.h"

static const char TAG[] = "wifi_app";

ESP_EVENT_DEFINE_BASE(WIFI_APP_EVENTS);

esp_event_loop_handle_t wifi_app_event_handle;

static int g_connection_retries = 0;

esp_netif_t *esp_netif_sta = NULL;
esp_netif_t *esp_netif_ap = NULL;

static void wifi_app_event_handler(void *args, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ESP_LOGI(TAG, "wifi_app_event_handler");
    if (event_base == WIFI_EVENT) {
        esp_err_t err;
        int8_t post_event_id = -1;

        switch (event_id) {
            case WIFI_EVENT_AP_START:
                ESP_LOGI(TAG, "WIFI_EVENT_AP_START");
                break;
            case WIFI_EVENT_AP_STOP:
                ESP_LOGI(TAG, "WIFI_EVENT_AP_STOP");
                break;
            case WIFI_EVENT_AP_STACONNECTED:
                ESP_LOGI(TAG, "WIFI_EVENT_AP_STACONNECTED");
                post_event_id = WIFI_APP_CONNECTED;
                break;
            case WIFI_EVENT_AP_STADISCONNECTED:
                ESP_LOGI(TAG, "WIFI_EVENT_AP_STADISCONNECTED");
                post_event_id = WIFI_APP_DISCONNECTED;
                ESP_LOGI(TAG, "STOPPING MDNS");
                silion_sim7200_stop_scanning();
                break;
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
                break;
            case WIFI_EVENT_STA_STOP:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_STOP");
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
                post_event_id = WIFI_APP_CONNECTED;
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
                post_event_id = WIFI_APP_DISCONNECTED;
                silion_sim7200_stop_scanning();
                if (g_connection_retries < MAX_CONNECTION_RETRIES) {
                    uint8_t delay_factor = g_connection_retries / 3;
                    ESP_LOGI(TAG, "Retrying to connect to WiFi in %dms", delay_factor * 250);
                    vTaskDelay(pdMS_TO_TICKS(delay_factor * 250));
                    esp_wifi_connect();
                    g_connection_retries++;
                }
                break;
            default:
                ESP_LOGW(TAG, "WIFI_EVENT: %ld", event_id);
        }

        if (post_event_id != -1) {
            esp_event_post_to(wifi_app_event_handle,
                              WIFI_APP_EVENTS,
                              post_event_id,
                              NULL,
                              0,
                              portMAX_DELAY);
            app_state_set_comm_idle_status(post_event_id == WIFI_APP_DISCONNECTED);
        }

    } else if (event_base == IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP:
                ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
                break;
            case IP_EVENT_AP_STAIPASSIGNED:
                ESP_LOGI(TAG, "IP_EVENT_AP_STAIPASSIGNED");
                break;
            default:
                ESP_LOGW(TAG, "IP_EVENT: %ld", event_id);
        }
    }
}

static void wifi_app_event_handler_init(void) {
    ESP_LOGI(TAG, "wifi_app_event_handler_init");
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_event_handler_instance_t instance_wifi_event;
    esp_event_handler_instance_t instance_ip_event;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL,
                                                        &instance_wifi_event));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL,
                                                        &instance_ip_event));
}

static void wifi_app_default_wifi_init(void) {
    ESP_LOGI(TAG, "wifi_app_default_wifi_init");
    ESP_ERROR_CHECK(esp_netif_init());

    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    esp_netif_sta = esp_netif_create_default_wifi_sta();
    esp_netif_ap = esp_netif_create_default_wifi_ap();
}

static void wifi_app_soft_ap_config(void) {
    ESP_LOGI(TAG, "wifi_app_soft_ap_config");

    wifi_config_t ap_config = {
            .ap = {
                    .ssid = {0},
                    .ssid_len = 0,
                    .password = WIFI_AP_PASSWORD,
                    .channel = WIFI_AP_CHANNEL,
                    .ssid_hidden = WIFI_AP_SSID_HIDDEN,
                    .authmode = WIFI_AUTH_WPA2_PSK,
                    .max_connection = WIFI_AP_MAX_CONNECTION,
                    .beacon_interval = WIFI_AP_BEACON_INTERVAL,
            },
    };

    strcpy((char *) ap_config.ap.ssid, settings.device_name);
    ap_config.ap.ssid_len = strlen(settings.device_name);

    esp_netif_ip_info_t ap_ip_info;
    memset(&ap_ip_info, 0, sizeof(ap_ip_info));

    esp_netif_dhcps_stop(esp_netif_ap);
    inet_pton(AF_INET, WIFI_AP_IP, &ap_ip_info.ip);
    inet_pton(AF_INET, WIFI_AP_GATEWAY, &ap_ip_info.gw);
    inet_pton(AF_INET, WIFI_AP_NETMASK, &ap_ip_info.netmask);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info));
    ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_AP, WIFI_AP_BANDWIDTH));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_STA_POWER_SAVE));
}

void wifi_app_sta_start(void) {
    ESP_LOGI(TAG, "wifi_app_sta_start");
    g_connection_retries = 0;

    wifi_config_t sta_config = {
            .sta = {
                    .ssid = {0},
                    .password = {0},
            },
    };

    strlcpy((char *) sta_config.sta.ssid, settings.network_ssid, strlen(settings.network_ssid));
    strlcpy((char *) sta_config.sta.password, settings.network_password, strlen(settings.network_password));

    ESP_LOGI(TAG, ".ssid %s", sta_config.sta.ssid);
    ESP_LOGI(TAG, ".password %s", sta_config.sta.password);

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
}

void wifi_app_ap_start(void) {
    ESP_LOGI(TAG, "wifi_app_ap_start");
    wifi_app_soft_ap_config();
}

void wifi_app_start(void) {
    esp_log_level_set("wifi", ESP_LOG_WARN);

    wifi_app_event_handler_init();

    wifi_app_default_wifi_init();

    ESP_ERROR_CHECK(esp_wifi_start());

    esp_event_loop_args_t loop_args = {
            .queue_size = WIFI_APP_TASK_QUEUE_SIZE,
            .task_name = WIFI_APP_TASK_NAME,
            .task_priority = WIFI_APP_TASK_PRIORITY,
            .task_stack_size = WIFI_APP_TASK_STACK_SIZE,
            .task_core_id = WIFI_APP_TASK_CORE_ID,
    };

    esp_event_loop_create(&loop_args, &wifi_app_event_handle);

    ESP_LOGI(TAG, "STARTING MDNS");
    int err = mdns_init();
    if (err) {
        ESP_LOGE(TAG, "MDNS Init failed: %d\n", err);
    }
    mdns_hostname_set(settings.device_name);
}

void wifi_app_stop(void) {

}
