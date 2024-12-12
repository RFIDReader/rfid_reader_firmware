#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_sleep.h"

#include "settings.h"
#include "buttons.h"
#include "app_state_disp.h"
#include "fan.h"
#include "buzzer.h"
#include "leds.h"
#include "ble_app.h"
#include "wifi_app.h"
#include "http_svr.h"
#include "comm_if.h"
#include "silion_sim7200.h"
#include "tasks_common.h"
#include "battery.h"
#include "usb_app.h"
#include "utils.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#include "tft_display.h"

#elif defined(CONFIG_IDF_TARGET_ESP32)

#include "oled_display.h"

#endif

static const char TAG[] = "rfid_reader";

void dispatch_to_op(void *pvParameter) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    app_state_dispatch_op_state();
    vTaskDelete(NULL);
}

void init(/*void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data*/) {
    switch (settings.comm_mode) {
        case COMM_MODE_BLE:
            ble_app_init();
            break;
        case COMM_MODE_WIFI_AP:
            wifi_app_start();
            wifi_app_ap_start();
            http_svr_start();
            break;
        case COMM_MODE_WIFI_STA:
            wifi_app_start();
            wifi_app_sta_start();
            http_svr_start();
            break;
        case COMM_MODE_USB:
            usb_app_init();
            ESP_LOGE(TAG, "usb comm_mode is not supported yet");
            break;
        default:
            ESP_LOGE(TAG, "unknown COMM_MODE: %d", settings.comm_mode);
    }

    comm_if_init();
    silion_sim7200_init();

    /// future: it will depend upon scanning status of the rfid reader
    fan_init();

    /// Depends upon:
    /// Comm Connection Status
    /// Button events
    /// Scanning Status of RFID reader
    buzzer_init();

    /// Depends upon:
    /// Comm Connection Status
    /// Button events
    /// Scanning Status of RFID reader
    leds_init();

    xTaskCreatePinnedToCore(dispatch_to_op,
                            "disp_to_op",
                            TEMP_TASK_STACK_SIZE,
                            NULL,
                            TEMP_TASK_PRIORITY,
                            NULL,
                            TEMP_TASK_CORE_ID);
}

void deinit() {

}

void warn_shutdown(/*void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data*/) {
    for (uint8_t i = 0; i < 30; ++i) {
        buzzer_on(25);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    buzzer_on(100);
    vTaskDelay(pdMS_TO_TICKS(100));
}

void handle_app_state(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    switch (event_id) {
        case APP_STATE_INITIALIZATION:
            init();
            break;
        case APP_STATE_OPERATIONAL:
            break;
        case APP_STATE_IDLE_WARNING:
            warn_shutdown();
            break;
        case APP_STATE_DEINITIALIZATION:
            deinit();
            break;
        case APP_STATE_SLEEPING:
            esp_sleep_enable_ext0_wakeup(OK_BTN_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(1));
            esp_deep_sleep_start();
    }
}

void app_main(void) {
    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    bms_init();
    buttons_init();

    settings_init();

    /// Depends upon:
    /// Button events
    app_state_init();
    vTaskDelay(pdMS_TO_TICKS(10));

#ifdef CONFIG_IDF_TARGET_ESP32S3
    /// Depends upon:
    /// Comm Connection Status
    /// Scanning Status of RFID reader
    /// Fan status
    /// Battery lvl & charging status
    tft_display_init();
#elif defined(CONFIG_IDF_TARGET_ESP32)
    oled_display_init();
#endif

    esp_event_handler_register_with(app_state_event_handle, APP_STATE_EVENTS, ESP_EVENT_ANY_ID, handle_app_state, NULL);
//    struct timeval t;
//    gettimeofday(&t, NULL);
//    ESP_LOGI(TAG, "system time: %lld", t.tv_sec);
    ESP_LOGI(TAG, ".comm_mode %d", settings.comm_mode);
    ESP_LOGI(TAG, ".network_ssid %s", settings.network_ssid);
    ESP_LOGI(TAG, ".network_password %s", settings.network_password);
    ESP_LOGI(TAG, ".button_notification %d", settings.button_notification);
    ESP_LOGI(TAG, ".alert_notification %d", settings.alert_notification);
    ESP_LOGI(TAG, ".tag_display_mode %d", settings.tag_display_mode);
    ESP_LOGI(TAG, ".device_name %s", settings.device_name);
    ESP_LOGI(TAG, ".btn_click_action %02X", settings.btn_click_action);
//    ESP_LOGI(TAG, ".btn_press_action: ");
//    print_hex_arr(settings.btn_press_action, settings.btn_press_action[0] + 1);
//    ESP_LOGI(TAG, ".btn_release_action: ");
//    print_hex_arr(settings.btn_release_action, settings.btn_release_action[0] + 1);
}