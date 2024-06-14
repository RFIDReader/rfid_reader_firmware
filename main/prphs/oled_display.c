//
// Created by Pratham Jaiswal on 11/05/24.
//

#include "oled_display.h"

#include "esp_log.h"
#include "esp_event.h"

#include "ssd1306.h"

#include "buttons.h"
#include "ble_app.h"
#include "wifi_app.h"
#include "settings.h"
#include "app_state_disp.h"

static const char TAG[] = "display";

static ssd1306_handle_t ssd1306 = NULL;

display_state_t state;

uint8_t calculate_menu_length(menu_t *menu);

void refresh_ui();

void navigate_back();

void comm_mode_set(uint8_t val) {
    settings_set_i8(COMM_MODE, val);
    navigate_back();
}

menu_t comm_mode_menu[] = {
        {
                .name = "1. BLE          ",
                .action = comm_mode_set,
                .settings_value = COMM_MODE_BLE,
        },
        {
                .name = "2. WiFi AP      ",
                .action = comm_mode_set,
                .settings_value = COMM_MODE_WIFI_AP,
        },
        {
                .name = "2. WiFi STA     ",
                .action = comm_mode_set,
                .settings_value = COMM_MODE_WIFI_STA,
        },
        {
                .name = "3. USB          ",
                .settings_value = COMM_MODE_USB,
        },
        {0},
};

void button_notification_set(uint8_t val) {
    settings_set_i8(BTN_NOTIFICATION, val);
    navigate_back();
}

menu_t button_notification_menu[] = {
        {
                .name = "1. LED          ",
                .action = button_notification_set,
                .settings_value = NOTIFICATION_TYPE_LED,
        },
        {
                .name = "2. BEEP         ",
                .action = button_notification_set,
                .settings_value = NOTIFICATION_TYPE_BEEP,
        },
        {
                .name = "3. LED + BEEP   ",
                .action = button_notification_set,
                .settings_value = NOTIFICATION_TYPE_LED_BEEP,
        },
        {0},
};


void alert_notification_set(uint8_t val) {
    settings_set_i8(ALERT_NOTIFICATION, val);
    navigate_back();
}

menu_t alert_notification_menu[] = {
        {
                .name = "1. LED          ",
                .action = alert_notification_set,
                .settings_value = NOTIFICATION_TYPE_LED,
        },
        {
                .name = "2. BEEP         ",
                .action = alert_notification_set,
                .settings_value = NOTIFICATION_TYPE_BEEP,
        },
        {
                .name = "3. LED + BEEP   ",
                .action = alert_notification_set,
                .settings_value = NOTIFICATION_TYPE_LED_BEEP,
        },
        {0},
};

menu_t notifications_menu[] = {
        {
                .name = "1. Button notify",
                .sub_menu = button_notification_menu,
                .settings_key = BTN_NOTIFICATION,
        },
        {
                .name = "2. Alert notify ",
                .sub_menu = alert_notification_menu,
                .settings_key = ALERT_NOTIFICATION,
        },
        {0},
};

void display_mode_set(uint8_t val) {
    settings_set_i8(SYSTEM_DISPLAY_MODE, val);
    navigate_back();
}

menu_t display_mode_menu[] = {
        {
                .name = "1. HEX          ",
                .action = display_mode_set,
                .settings_value = SYSTEM_DISPLAY_MODE_HEX,
        },
        {
                .name = "2. ASCII        ",
                .action = display_mode_set,
                .settings_value = SYSTEM_DISPLAY_MODE_ASCII,
        },
        {0},
};

menu_t system_setup_menu[] = {
        {
                .name = "1. Notifications",
                .sub_menu = notifications_menu,
        },
        {
                .name = "2. Display mode ",
                .sub_menu = display_mode_menu,
                .settings_key = SYSTEM_DISPLAY_MODE,
        },
        {0},
};

void main_menu_exit(uint8_t val) {
    app_state_dispatch_init_state();
}

void factory_reset(uint8_t val) {
    // TODO: reset all saved settings
    esp_restart();
}

menu_t main_menu[] = {
        {
                .name = "1. Comm mode    ",
                .sub_menu = comm_mode_menu,
                .settings_key = COMM_MODE,
        },
        {
                .name = "3. Network setup",
        },
        {
                .name = "4. System setup ",
                .sub_menu = system_setup_menu,
        },
        {
                .name = "6. Factory reset",
                .action = factory_reset,
        },
        {
                .name = "7. Exit         ",
                .action = main_menu_exit,
        },
        {
                .name = "8. F/w Version  ",
        },
        {0},
};

menu_t *current_menu;
menu_history_t history[5];
int8_t history_top = -1;

uint8_t calculate_menu_length(menu_t *menu) {
    if (!menu)
        return 0;
    uint8_t i = 0;
    for (; menu[i].name; ++i);
    return i;
}

void refresh_ui() {
    char lines[4][17];

    ssd1306_clear_screen(ssd1306, 0x00);

    switch (app_state) {
        case APP_STATE_STARTUP:
            snprintf(lines[0], sizeof(lines[0]), "    Booting     ");
            ssd1306_draw_string(ssd1306, 0, 24, (const uint8_t *) lines[0], 16, 1);
            break;
        case APP_STATE_CONFIGURATION: {
            uint8_t page = state.current_menu_focus_pos / 4;
            for (uint8_t i = (page * 4); (i < ((page + 1) * 4) && i < state.current_menu_len); ++i) {
                snprintf(lines[i % 4], sizeof(lines[i % 4]), "%s", current_menu[i].name);
                ssd1306_draw_string(ssd1306, 0, (i % 4) * 16, (const uint8_t *) lines[i % 4], 16,
                                    state.current_menu_focus_pos != i);
            }
            break;
        }
        case APP_STATE_INITIALIZATION:
            snprintf(lines[0], sizeof(lines[0]), "%c %c   ----   %d%%",
                     settings.comm_mode == COMM_MODE_BLE ? 'B' : (settings.comm_mode == COMM_MODE_WIFI_AP ||
                                                                  settings.comm_mode == COMM_MODE_WIFI_STA) ? 'W'
                                                                                                            : '?',
                     state.client_connected ? '*' : '?', 75);
            snprintf(lines[1], sizeof(lines[1]), "      Wait      ");
            ssd1306_draw_string(ssd1306, 0, 0, (const uint8_t *) lines[0], 16, 1);
            ssd1306_draw_string(ssd1306, 0, 24, (const uint8_t *) lines[1], 16, 1);
            break;
        case APP_STATE_OPERATIONAL:
            snprintf(lines[0], sizeof(lines[0]), "%c %c   ----   %d%%",
                     settings.comm_mode == COMM_MODE_BLE ? 'B' : (settings.comm_mode == COMM_MODE_WIFI_AP ||
                                                                  settings.comm_mode == COMM_MODE_WIFI_STA) ? 'W'
                                                                                                            : '?',
                     state.client_connected ? '*' : '?', 75);
            snprintf(lines[1], sizeof(lines[1]), "   Scan Ready   ");
            ssd1306_draw_string(ssd1306, 0, 0, (const uint8_t *) lines[0], 16, 1);
            ssd1306_draw_string(ssd1306, 0, 24, (const uint8_t *) lines[1], 16,
                                1); // can be set to 16 in order to show at second row
            break;
    }

    ssd1306_refresh_gram(ssd1306);
}

void display_handle_ble_events(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    switch (id) {
        case BLE_EVENT_CONNECTED:
            ESP_LOGI(TAG, "BLUETOOTH_EVENT_CLIENT_CONNECTED");
            state.client_connected = true;
            refresh_ui();
            break;
        case BLE_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "BLUETOOTH_EVENT_CLIENT_DISCONNECTED");
            state.client_connected = false;
            refresh_ui();
            break;
        default:
            break;
    }
}

void display_handle_wifi_events(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    switch (id) {
        case WIFI_APP_CONNECTED:
            state.client_connected = true;
            refresh_ui();
            break;
        case WIFI_APP_DISCONNECTED:
            state.client_connected = false;
            refresh_ui();
            break;
        default:
            break;
    }
}

void display_handle_app_state_events(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    switch (id) {
        case APP_STATE_STARTUP:
            break;
        case APP_STATE_CONFIGURATION:
            break;
        case APP_STATE_INITIALIZATION:
            break;
        case APP_STATE_OPERATIONAL:
            if (settings.comm_mode == COMM_MODE_BLE) {
                esp_event_handler_register_with(ble_event_handle,
                                                BLE_APP_EVENTS,
                                                ESP_EVENT_ANY_ID,
                                                display_handle_ble_events,
                                                NULL);
            } else if ((settings.comm_mode == COMM_MODE_WIFI_AP || settings.comm_mode == COMM_MODE_WIFI_STA)) {
                esp_event_handler_register_with(wifi_app_event_handle,
                                                WIFI_APP_EVENTS,
                                                ESP_EVENT_ANY_ID,
                                                display_handle_wifi_events,
                                                NULL);
            }
            break;
        default:
            break;
    }
    refresh_ui();
}

void display_handle_button_events(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    switch (id) {
        case NEXT_BUTTON_PRESSED:
            if (state.current_menu_focus_pos < state.current_menu_len - 1) {
                state.current_menu_focus_pos++;
            } else {
                state.current_menu_focus_pos = 0;
            }
            break;
        case PREV_BUTTON_PRESSED:
            if (state.current_menu_focus_pos > 0) {
                state.current_menu_focus_pos--;
            } else {
                state.current_menu_focus_pos = state.current_menu_len - 1;
            }
            break;
        case OK_BUTTON_PRESSED:
            if (current_menu[state.current_menu_focus_pos].action) {
                current_menu[state.current_menu_focus_pos].action(
                        current_menu[state.current_menu_focus_pos].settings_value);
            } else if (current_menu[state.current_menu_focus_pos].sub_menu) {
                push_menu(history, current_menu, state.current_menu_focus_pos);
                int8_t val = -1;
                if (current_menu[state.current_menu_focus_pos].settings_key) {
                    val = settings_get_i8(current_menu[state.current_menu_focus_pos].settings_key);
                }
                current_menu = current_menu[state.current_menu_focus_pos].sub_menu;
                state.current_menu_len = calculate_menu_length(current_menu);
                bool focus_set = false;
                if (val != -1) {
                    for (int i = 0; i < state.current_menu_len; ++i) {
                        if (current_menu[i].settings_value == val) {
                            state.current_menu_focus_pos = i;
                            focus_set = true;
                            break;
                        }
                    }
                }
                if (!focus_set) {
                    state.current_menu_focus_pos = 0;
                }
            }
            break;
        case C_BUTTON_PRESSED:
            navigate_back();
            break;
        default:
            break;
    }
    refresh_ui();
}

void oled_display_init() {
    ESP_LOGI(TAG, "display_init");
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t) OLED_SDA_GPIO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t) OLED_SCL_GPIO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 1000000;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);

    ssd1306 = ssd1306_create(I2C_NUM_0, SSD1306_I2C_ADDRESS);
    ssd1306_refresh_gram(ssd1306);
    ssd1306_clear_screen(ssd1306, 0x00);

    current_menu = main_menu;
    state.current_menu_len = calculate_menu_length(current_menu);
    state.current_menu_focus_pos = 0;

    refresh_ui();

    esp_event_handler_register_with(app_state_event_handle,
                                    APP_STATE_EVENTS,
                                    ESP_EVENT_ANY_ID,
                                    display_handle_app_state_events,
                                    NULL);

    esp_event_handler_register_with(button_event_handle,
                                    BUTTON_EVENTS,
                                    ESP_EVENT_ANY_ID,
                                    display_handle_button_events,
                                    NULL);
}

void navigate_back() {
    menu_history_t *last_history = pop_menu(history);
    if (last_history) {
        current_menu = last_history->menu;
        state.current_menu_focus_pos = last_history->focus_pos;
        state.current_menu_len = calculate_menu_length(current_menu);
    }
    refresh_ui();
}

void push_menu(menu_history_t *history, menu_t *menu, uint8_t focus_pos) {
    if (history_top < 3) {
        history_top++;
        history[history_top].menu = menu;
        history[history_top].focus_pos = focus_pos;
    }
}

menu_history_t *pop_menu(menu_history_t *history) {
    if (history_top >= 0) {
        return &history[history_top--];
    }
    return NULL;
}


