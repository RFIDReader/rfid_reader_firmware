//
// Created by Pratham Jaiswal on 20/03/24.
//

#include <nvs.h>
#include <esp_log.h>
#include <string.h>
#include <stdint.h>

#include "settings.h"
#include "utils.h"

static const char TAG[] = "settings";

settings_t settings;

nvs_handle_t settings_handle;

void settings_init() {
    esp_err_t err = nvs_open("settings", NVS_READWRITE, &settings_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "error opening nvs handle %s", esp_err_to_name(err));
    } else {
        int8_t comm_mode = COMM_MODE_BLE;
        err = nvs_get_i8(settings_handle, COMM_MODE, &comm_mode);
        switch (err) {
            case ESP_OK:
                settings.comm_mode = comm_mode;
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGI(TAG, "%s not found. setting it to %d", COMM_MODE, comm_mode);
                settings_set_i8(COMM_MODE, comm_mode);
                break;
            default:
                ESP_LOGE(TAG, "error reading value %s", esp_err_to_name(err));
        }

        char *network_ssid = (char *) malloc(32 * sizeof(char));
        size_t network_ssid_size = 32;
        err = nvs_get_str(settings_handle, NETWORK_SETUP_WIFI_STA_SSID, network_ssid, &network_ssid_size);
        switch (err) {
            case ESP_OK:
                settings.network_ssid = network_ssid;
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                settings_set_str(NETWORK_SETUP_WIFI_STA_SSID, "pr47h4m");
                break;
            default:
                ESP_LOGE(TAG, "error reading value %s", esp_err_to_name(err));
        }

        char *network_password = (char *) malloc(64 * sizeof(char));
        size_t network_password_size = 64;
        err = nvs_get_str(settings_handle, NETWORK_SETUP_WIFI_STA_PSWD, network_password, &network_password_size);
        switch (err) {
            case ESP_OK:
                settings.network_password = network_password;
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                settings_set_str(NETWORK_SETUP_WIFI_STA_PSWD, "password");
                break;
            default:
                ESP_LOGE(TAG, "error reading value %s", esp_err_to_name(err));
        }

        int8_t button_notification = NOTIFICATION_TYPE_LED_BEEP;
        err = nvs_get_i8(settings_handle, BTN_NOTIFICATION, &button_notification);
        switch (err) {
            case ESP_OK:
                settings.button_notification = button_notification;
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                settings_set_i8(BTN_NOTIFICATION, button_notification);
                break;
            default:
                ESP_LOGE(TAG, "error reading value %s", esp_err_to_name(err));
        }

        int8_t alert_notification = NOTIFICATION_TYPE_LED_BEEP;
        err = nvs_get_i8(settings_handle, ALERT_NOTIFICATION, &alert_notification);
        switch (err) {
            case ESP_OK:
                settings.alert_notification = alert_notification;
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                settings_set_i8(ALERT_NOTIFICATION, alert_notification);
                break;
            default:
                ESP_LOGE(TAG, "error reading value %s", esp_err_to_name(err));
        }

        int8_t tag_display_mode = SYSTEM_DISPLAY_MODE_HEX;
        err = nvs_get_i8(settings_handle, SYSTEM_DISPLAY_MODE, &tag_display_mode);
        switch (err) {
            case ESP_OK:
                settings.tag_display_mode = tag_display_mode;
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                settings_set_i8(SYSTEM_DISPLAY_MODE, tag_display_mode);
                break;
            default:
                ESP_LOGE(TAG, "error reading value %s", esp_err_to_name(err));
        }

        uint8_t mac_addr[6];
        get_mac_addr(mac_addr, mac_addr, mac_addr);

        char *device_name = (char *) malloc(32 * sizeof(char));
        size_t device_name_size = 32;
        sprintf(device_name, "RfidReader-%02X%02X", mac_addr[4], mac_addr[5]);
        err = nvs_get_str(settings_handle, SYSTEM_DEVICE_NAME, device_name, &device_name_size);
        switch (err) {
            case ESP_OK:
                settings.device_name = device_name;
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                settings_set_str(SYSTEM_DEVICE_NAME, device_name);
                settings.device_name = device_name;
                break;
            default:
                ESP_LOGE(TAG, "error reading value %s", esp_err_to_name(err));
        }

        int8_t btn_click_action = SYSTEM_BTN_CLICK_NO_ACTION;
        err = nvs_get_i8(settings_handle, SYSTEM_BTN_CLICK_ACTION, &btn_click_action);
        switch (err) {
            case ESP_OK:
                settings.btn_click_action = btn_click_action;
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                settings_set_i8(SYSTEM_BTN_CLICK_ACTION, btn_click_action);
                break;
            default:
                ESP_LOGE(TAG, "error reading value %s", esp_err_to_name(err));
        }

        uint8_t *btn_press_action = (uint8_t *) malloc(255 * sizeof(uint8_t));
        size_t btn_press_action_size = 5;
        btn_press_action[0] = 0xFF;
        btn_press_action[1] = 0x00;
        btn_press_action[2] = 0x03;
        btn_press_action[3] = 0x1D;
        btn_press_action[4] = 0x0C;
        err = nvs_get_blob(settings_handle, SYSTEM_BTN_PRESS_ACTION, btn_press_action, &btn_press_action_size);
        switch (err) {
            case ESP_OK:
                settings.btn_press_action = btn_press_action;
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                settings_set_blob(SYSTEM_BTN_PRESS_ACTION, btn_press_action, btn_press_action_size);
                settings.btn_press_action = btn_press_action;
                break;
            default:
                ESP_LOGE(TAG, "error reading value %s", esp_err_to_name(err));
        }

        uint8_t *btn_release_action = (uint8_t *) malloc(255 * sizeof(uint8_t));
        size_t btn_release_action_size = 5;
        btn_release_action[0] = 0xFF;
        btn_release_action[1] = 0x00;
        btn_release_action[2] = 0x03;
        btn_release_action[3] = 0x1D;
        btn_release_action[4] = 0x0C;
        err = nvs_get_blob(settings_handle, SYSTEM_BTN_RELEASE_ACTION, btn_release_action, &btn_release_action_size);
        switch (err) {
            case ESP_OK:
                settings.btn_release_action = btn_release_action;
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                settings_set_blob(SYSTEM_BTN_RELEASE_ACTION, btn_release_action, btn_press_action_size);
                settings.btn_release_action = btn_release_action;
                break;
            default:
                ESP_LOGE(TAG, "error reading value %s", esp_err_to_name(err));
        }
    }
}

void settings_set_blob(char *key, uint8_t *val, uint8_t len) {
    uint8_t new_val[len + 1];
    new_val[0] = len;
    memcpy(&new_val[1], val, len);

    esp_err_t err = nvs_set_blob(settings_handle, key, new_val, len + 1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "error writing value %s", esp_err_to_name(err));
    }
    if (strcmp(key, SYSTEM_BTN_PRESS_ACTION) == 0) {
        settings.btn_press_action = val;
    } else if (strcmp(key, SYSTEM_BTN_RELEASE_ACTION) == 0) {
        settings.btn_release_action = val;
    }
}

uint8_t *settings_get_blob(char *key) {
    size_t val_len;
    esp_err_t err = nvs_get_blob(settings_handle, key, NULL, &val_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "error getting value %s", esp_err_to_name(err));
    }
    uint8_t *val = (uint8_t *) malloc((val_len) * sizeof(uint8_t));
    nvs_get_blob(settings_handle, key, val, &val_len);
    return val;
}

void settings_set_i8(char *key, uint8_t val) {
    esp_err_t err = nvs_set_i8(settings_handle, key, val);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "error writing value %s", esp_err_to_name(err));
    }
    if (strcmp(key, COMM_MODE) == 0) {
        settings.comm_mode = val;
    } else if (strcmp(key, BTN_NOTIFICATION) == 0) {
        settings.button_notification = val;
    } else if (strcmp(key, ALERT_NOTIFICATION) == 0) {
        settings.alert_notification = val;
    } else if (strcmp(key, SYSTEM_DISPLAY_MODE) == 0) {
        settings.tag_display_mode = val;
    }
}

int8_t settings_get_i8(char *key) {
    int8_t val = -1;
    esp_err_t err = nvs_get_i8(settings_handle, key, &val);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "error reading value %s", esp_err_to_name(err));
        return -1;
    }
    return val;
}

void settings_set_str(char *key, char *val) {
    esp_err_t err = nvs_set_str(settings_handle, key, val);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "error writing value %s", esp_err_to_name(err));
    }
    if (strcmp(key, NETWORK_SETUP_WIFI_STA_SSID) == 0) {
        settings.network_ssid = val;
    } else if (strcmp(key, NETWORK_SETUP_WIFI_STA_PSWD) == 0) {
        settings.network_password = val;
    } else if (strcmp(key, SYSTEM_DEVICE_NAME) == 0) {
        settings.device_name = val;
    }
}

char *settings_get_str(char *key) {
    size_t val_len;
    esp_err_t err = nvs_get_str(settings_handle, key, NULL, &val_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "error getting value %s", esp_err_to_name(err));
    }
    char *val = (char *) malloc((val_len + 1) * sizeof(char));
    nvs_get_str(settings_handle, key, val, &val_len);
    val[val_len] = '\0';
    return val;
}

void settings_reset() {
    nvs_erase_key(settings_handle, COMM_MODE);
    nvs_erase_key(settings_handle, NETWORK_SETUP_WIFI_STA_SSID);
    nvs_erase_key(settings_handle, NETWORK_SETUP_WIFI_STA_PSWD);
    nvs_erase_key(settings_handle, BTN_NOTIFICATION);
    nvs_erase_key(settings_handle, ALERT_NOTIFICATION);
    nvs_erase_key(settings_handle, SYSTEM_DISPLAY_MODE);
    nvs_erase_key(settings_handle, SYSTEM_DEVICE_NAME);
}