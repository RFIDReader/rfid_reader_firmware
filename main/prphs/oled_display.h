//
// Created by Pratham Jaiswal on 11/05/24.
//

#ifndef RFID_READER_OLED_DISPLAY_H
#define RFID_READER_OLED_DISPLAY_H

#include <stdint.h>
#include "driver/gpio.h"

/** Scanner UI - 128x64
+----------------+
|B ?   ---   ---%| <- row #1 => y = 0
|                | <- row #2 => y = 16
|                | <- row #3 => y = 32
|                | <- row #4 => y = 48
+----------------+

+----------------+
|B L   28C   100%|
|   Scan Ready   |
|                |
|                |
+----------------+
 */

#ifdef CONFIG_IDF_TARGET_ESP32

#define OLED_SDA_GPIO GPIO_NUM_21
#define OLED_SCL_GPIO GPIO_NUM_22

#elif defined(CONFIG_IDF_TARGET_ESP32S3)

#define OLED_SDA_GPIO GPIO_NUM_3
#define OLED_SCL_GPIO GPIO_NUM_2

#endif

typedef struct {
    uint8_t client_connected;
    uint8_t current_menu_len;
    uint8_t current_menu_focus_pos;
} display_state_t;

typedef void (*action_t)(uint8_t);

typedef struct menu {
    char *name;
    action_t action;
    uint8_t settings_value;
    char *settings_key;
    struct menu *sub_menu;
} menu_t;

typedef struct {
    menu_t *menu;
    uint8_t focus_pos;
} menu_history_t;

void push_menu(menu_history_t *history, menu_t *menu, uint8_t focus_pos);

menu_history_t *pop_menu(menu_history_t *history);

void oled_display_init();

#endif //RFID_READER_OLED_DISPLAY_H
