//
// Created by Pratham Jaiswal on 20/03/24.
//

#ifndef RFID_READER_WIFI_APP_H
#define RFID_READER_WIFI_APP_H

#include <esp_netif.h>
#include "esp_event.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3


#elif defined(CONFIG_IDF_TARGET_ESP32)


#endif

ESP_EVENT_DECLARE_BASE(WIFI_APP_EVENTS);

enum {
    WIFI_APP_DISCONNECTED = 0,
    WIFI_APP_CONNECTED
};

extern esp_event_loop_handle_t wifi_app_event_handle;

#define WIFI_AP_PASSWORD "password"
#define WIFI_AP_CHANNEL 1
#define WIFI_AP_SSID_HIDDEN 0
#define WIFI_AP_MAX_CONNECTION 3
#define WIFI_AP_BEACON_INTERVAL 100
#define WIFI_AP_IP "192.168.0.1"
#define WIFI_AP_GATEWAY "192.168.0.1"
#define WIFI_AP_NETMASK "255.255.255.0"
#define WIFI_AP_BANDWIDTH WIFI_BW_HT40
#define WIFI_STA_POWER_SAVE WIFI_PS_NONE
#define MAX_CONNECTION_RETRIES 30

extern esp_netif_t *esp_netif_sta;
extern esp_netif_t *esp_netif_ap;

void wifi_app_start(void),
        wifi_app_ap_start(void),
        wifi_app_sta_start(void);

#endif //RFID_READER_WIFI_APP_H
