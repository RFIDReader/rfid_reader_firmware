//
// Created by Pratham Jaiswal on 20/03/24.
//

#ifndef RFID_READER_BUZZER_H
#define RFID_READER_BUZZER_H

#include "driver/gpio.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#define BUZZER_GPIO GPIO_NUM_1

#elif defined(CONFIG_IDF_TARGET_ESP32)

#define BUZZER_GPIO GPIO_NUM_4

#endif

void buzzer_init(void);

void buzzer_on(uint32_t duration_ms);

//void buzzer_play_melody(uint16_t *notes, uint32_t *durations, uint16_t length, uint8_t slow);

#endif //RFID_READER_BUZZER_H
