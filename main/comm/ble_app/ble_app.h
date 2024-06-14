//
// Created by Pratham Jaiswal on 20/03/24.
//

#ifndef RFID_READER_BLE_APP_H
#define RFID_READER_BLE_APP_H

#include "esp_event.h"
#include "types.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#define USE_2M_PHY

#elif defined(CONFIG_IDF_TARGET_ESP32)


#endif

ESP_EVENT_DECLARE_BASE(BLE_APP_EVENTS);

enum {
    BLE_EVENT_CONNECTED,
    BLE_EVENT_DISCONNECTED
};

extern esp_event_loop_handle_t ble_event_handle;

void ble_app_init(void),
        ble_app_send_msg(msg_t *msg);

#endif //RFID_READER_BLE_APP_H
