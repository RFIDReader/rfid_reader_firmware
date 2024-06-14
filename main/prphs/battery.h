//
// Created by Pratham Jaiswal on 10/04/24.
//

#ifndef RFID_READER_BATTERY_H
#define RFID_READER_BATTERY_H

#include "esp_event.h"
#include "driver/gpio.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#define FUEL_GAUGE_SDA GPIO_NUM_3
#define FUEL_GAUGE_SCL GPIO_NUM_2
#define FUEL_GAUGE_GPIO GPIO_NUM_18

#elif defined(CONFIG_IDF_TARGET_ESP32)

// BMS is not present

#endif

extern uint8_t bat_lvl;

void bms_init(void);

#endif //RFID_READER_BATTERY_H
