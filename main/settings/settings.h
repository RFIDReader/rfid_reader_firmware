//
// Created by Pratham Jaiswal on 20/03/24.
//

#ifndef RFID_READER_SETTINGS_H
#define RFID_READER_SETTINGS_H

#include <stdint.h>

#define COMM_MODE "commod"
#define COMM_MODE_BLE 0
// Rfid-Reader acts as Wi-Fi AP
#define COMM_MODE_WIFI_AP 1
// Rfid-Reader acts as Wi-Fi STA
#define COMM_MODE_WIFI_STA 2
#define COMM_MODE_USB 3

#define NETWORK_SETUP_WIFI_STA_SSID "net.sta.ssid"
#define NETWORK_SETUP_WIFI_STA_PSWD "net.sta.pswd"

#define BTN_NOTIFICATION "noti.btn"
#define ALERT_NOTIFICATION "noti.alert"
#define NOTIFICATION_TYPE_LED 0
#define NOTIFICATION_TYPE_BEEP 1
#define NOTIFICATION_TYPE_LED_BEEP 2

#define SYSTEM_DISPLAY_MODE "sys.disp.tag"
#define SYSTEM_DISPLAY_MODE_HEX 0
#define SYSTEM_DISPLAY_MODE_ASCII 1

#define SYSTEM_DEVICE_NAME "sys.name"

typedef struct {
    uint8_t comm_mode;
    char *network_ssid;
    char *network_password;
    uint8_t button_notification;
    uint8_t alert_notification;
    uint8_t tag_display_mode;
    char *device_name;
} settings_t;

extern settings_t settings;

void settings_init(void);

void settings_set_i8(char *key, uint8_t val);

int8_t settings_get_i8(char *key);

void settings_set_str(char *key, char *val);

char *settings_get_str(char *key);

void settings_reset(void);

#endif //RFID_READER_SETTINGS_H
