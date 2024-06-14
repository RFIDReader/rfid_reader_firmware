//
// Created by Pratham Jaiswal on 20/03/24.
//

#ifndef RFID_READER_TFT_DISPLAY_H
#define RFID_READER_TFT_DISPLAY_H

#include "driver/gpio.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#define MOSI_GPIO GPIO_NUM_39
#define SCL_GPIO GPIO_NUM_38
#define CS_GPIO GPIO_NUM_41
#define DC_GPIO GPIO_NUM_40
#define BLK_GPIO GPIO_NUM_47
#define RST_GPIO (-1)

#elif defined(CONFIG_IDF_TARGET_ESP32)

// Uses 0.96" SSD1306 driver display module
// Dynamic display is not supported
// Currently its programmed in oled_display.c

#endif

void tft_display_init(void);

#endif //RFID_READER_TFT_DISPLAY_H
