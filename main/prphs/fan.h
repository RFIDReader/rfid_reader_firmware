//
// Created by Pratham Jaiswal on 20/03/24.
//

#ifndef RFID_READER_FAN_H
#define RFID_READER_FAN_H

#include "esp_event.h"
#include "driver/gpio.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#define FAN_SUPPORTED
#define FAN_GPIO GPIO_NUM_8

#elif defined(CONFIG_IDF_TARGET_ESP32)

// Fan is not supported

#endif

static bool fan_state;

void fan_init(void);

#endif //RFID_READER_FAN_H
