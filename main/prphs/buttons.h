//
// Created by Pratham Jaiswal on 20/03/24.
//

#ifndef RFID_READER_BUTTONS_H
#define RFID_READER_BUTTONS_H

#include "esp_event.h"
#include "driver/gpio.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#define SCAN_BTN_GPIO GPIO_NUM_12
#define C_BTN_GPIO GPIO_NUM_13
#define OK_BTN_GPIO GPIO_NUM_14
#define PREV_BTN_GPIO GPIO_NUM_10
#define NEXT_BTN_GPIO GPIO_NUM_11

#elif defined(CONFIG_IDF_TARGET_ESP32)

#define SCAN_BTN_GPIO GPIO_NUM_25
#define C_BTN_GPIO GPIO_NUM_33
#define OK_BTN_GPIO GPIO_NUM_26
#define PREV_BTN_GPIO GPIO_NUM_27
#define NEXT_BTN_GPIO GPIO_NUM_32

#endif

ESP_EVENT_DECLARE_BASE(BUTTON_EVENTS);

enum {
    NEXT_BUTTON_PRESSED = 0,
    PREV_BUTTON_PRESSED,
    SCAN_BUTTON_PRESSED,
    SCAN_BUTTON_HOLD,
    SCAN_BUTTON_RELEASED,
    C_BUTTON_PRESSED,
    OK_BUTTON_PRESSED,
    OK_BUTTON_HOLD,
};

extern esp_event_loop_handle_t button_event_handle;

void buttons_init(void);

#endif //RFID_READER_BUTTONS_H
