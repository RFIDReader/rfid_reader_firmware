//
// Created by Pratham Jaiswal on 20/03/24.
//

#ifndef RFID_READER_LEDS_H
#define RFID_READER_LEDS_H

#include "driver/gpio.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#define USE_LED_STRIP
#define CON_INDI_GPIO GPIO_NUM_42
#define NOTIFICATION_LED_GPIO GPIO_NUM_7

#elif defined(CONFIG_IDF_TARGET_ESP32)

#define USE_RGB
#define RGB_RED_GPIO GPIO_NUM_5
#define RGB_GREEN_GPIO GPIO_NUM_18
#define RGB_BLUE_GPIO GPIO_NUM_19

#define NOTIFICATION_LED_GPIO GPIO_NUM_14

#define USE_POW
#define POWER_LED_GPIO GPIO_NUM_23

#endif

void leds_init(void);

#endif //RFID_READER_LEDS_H
