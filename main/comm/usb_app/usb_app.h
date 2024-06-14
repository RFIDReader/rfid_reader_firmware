//
// Created by Pratham Jaiswal on 20/03/24.
//

#ifndef RFID_READER_USB_APP_H
#define RFID_READER_USB_APP_H

#include "esp_event.h"
#include "driver/gpio.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#define USBD_P_GPIO GPIO_NUM_20
#define USBD_N_GPIO GPIO_NUM_19

#elif defined(CONFIG_IDF_TARGET_ESP32)

// USB Comm is not supported

#endif

ESP_EVENT_DECLARE_BASE(USB_APP_EVENTS);

enum {
    USB_APP_DISCONNECTED = 0,
    USB_APP_CONNECTED
};

extern esp_event_loop_handle_t usb_app_event_handle;

void usb_app_init();

#endif //RFID_READER_USB_APP_H
