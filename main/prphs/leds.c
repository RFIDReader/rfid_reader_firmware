//
// Created by Pratham Jaiswal on 20/03/24.
//

#include <esp_event.h>
#include <esp_log.h>

#include "leds.h"
#include "ble_app.h"
#include "wifi_app.h"
#include "usb_app.h"
#include "settings.h"
#include "buttons.h"
#include "comm_if.h"
#include "tasks_common.h"

#ifdef USE_LED_STRIP

#include "led_strip.h"

led_strip_handle_t rgb_handle;

led_strip_config_t rgb_cfg = {
        .strip_gpio_num = CON_INDI_GPIO,
        .max_leds = 1,
};

led_strip_rmt_config_t rgb_rmt_cfg = {
        .resolution_hz =10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
};

#endif

typedef struct {
    int r;
    int g;
    int b;
    int on_time_ms;
    int off_time_ms;
} blink_settings_t;

TaskHandle_t blink_task_handle;

static void blink_task(void *pvParameters) {
    blink_settings_t *blink_settings = (blink_settings_t *) pvParameters;
    for (;;) {
#ifdef USE_LED_STRIP
        led_strip_set_pixel(rgb_handle, 0, blink_settings->r, blink_settings->g, blink_settings->b);
        led_strip_refresh(rgb_handle);
        vTaskDelay(pdMS_TO_TICKS(blink_settings->on_time_ms));
        led_strip_clear(rgb_handle);
        vTaskDelay(pdMS_TO_TICKS(blink_settings->off_time_ms));
#elif defined USE_RGB
        gpio_set_level(RGB_RED_GPIO, blink_settings->r & 255);
        gpio_set_level(RGB_GREEN_GPIO, blink_settings->g & 255);
        gpio_set_level(RGB_BLUE_GPIO, blink_settings->b & 255);
        vTaskDelay(pdMS_TO_TICKS(blink_settings->on_time_ms));
        gpio_set_level(RGB_RED_GPIO, 0);
        gpio_set_level(RGB_GREEN_GPIO, 0);
        gpio_set_level(RGB_BLUE_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(blink_settings->off_time_ms));
#endif
    }
}

void blink_notification_led() {
    gpio_set_direction(NOTIFICATION_LED_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_direction(NOTIFICATION_LED_GPIO, 0);
}

void leds_handle_ble_events(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    blink_settings_t *blink_settings = (blink_settings_t *) malloc(sizeof(blink_settings_t));
    blink_settings->r = 0;
    blink_settings->g = 0;
    blink_settings->b = 255;
    blink_settings->on_time_ms = 100;
    blink_settings->off_time_ms = event_id == BLE_EVENT_CONNECTED ? 3000 : 1000;

    if (blink_task_handle) {
        vTaskDelete(blink_task_handle);
        blink_task_handle = NULL;
    }
    xTaskCreatePinnedToCore(blink_task,
                            "blink_tsk",
                            TEMP_TASK_STACK_SIZE,
                            (void *) &blink_settings,
                            TEMP_TASK_PRIORITY,
                            &blink_task_handle,
                            TEMP_TASK_CORE_ID);
}

void leds_handle_wifi_events(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ESP_LOGI("leds", "received wifi event %ld", event_id);
    blink_settings_t *blink_settings = (blink_settings_t *) malloc(sizeof(blink_settings_t));
    blink_settings->r = 0;
    blink_settings->g = 255;
    blink_settings->b = 0;
    blink_settings->on_time_ms = 100;
    blink_settings->off_time_ms = event_id == WIFI_APP_CONNECTED ? 3000 : 1000;

    if (blink_task_handle) {
        vTaskDelete(blink_task_handle);
        blink_task_handle = NULL;
    }
    xTaskCreatePinnedToCore(blink_task,
                            "blink_tsk",
                            TEMP_TASK_STACK_SIZE,
                            (void *) blink_settings,
                            TEMP_TASK_PRIORITY,
                            &blink_task_handle,
                            TEMP_TASK_CORE_ID);
}

void leds_handle_usb_events(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    blink_settings_t *blink_settings = (blink_settings_t *) malloc(sizeof(blink_settings_t));
    blink_settings->r = 255;
    blink_settings->g = 0;
    blink_settings->b = 0;
    blink_settings->on_time_ms = 100;
    blink_settings->off_time_ms = event_id == USB_APP_CONNECTED ? 3000 : 1000;

    if (blink_task_handle) {
        vTaskDelete(blink_task_handle);
        blink_task_handle = NULL;
    }
    xTaskCreatePinnedToCore(blink_task,
                            "blink_tsk",
                            TEMP_TASK_STACK_SIZE,
                            (void *) &blink_settings,
                            TEMP_TASK_PRIORITY,
                            &blink_task_handle,
                            TEMP_TASK_CORE_ID);
}

void leds_handle_comm_events(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_id == COMM_EVENT_TAG_READ || event_id == COMM_EVENT_HEARTBEAT_RECEIVED) {
        blink_notification_led();
    }
}

void leds_handle_btn_events(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    blink_notification_led();
}

void leds_init(void) {
#ifdef USE_LED_STRIP
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&rgb_cfg, &rgb_rmt_cfg, &rgb_handle));
#elif defined USE_RGB
    gpio_set_direction(RGB_RED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(RGB_GREEN_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(RGB_BLUE_GPIO, GPIO_MODE_OUTPUT);
#endif

    gpio_set_direction(NOTIFICATION_LED_GPIO, GPIO_MODE_OUTPUT);

#ifdef USE_POW
    gpio_set_direction(POWER_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(POWER_LED_GPIO, 1);
#endif

    if (ble_event_handle) {
        leds_handle_ble_events(NULL, BLE_APP_EVENTS, BLE_EVENT_DISCONNECTED, NULL);
        esp_event_handler_register_with(ble_event_handle,
                                        BLE_APP_EVENTS,
                                        ESP_EVENT_ANY_ID,
                                        leds_handle_ble_events,
                                        NULL);
    } else if (wifi_app_event_handle) {
        leds_handle_wifi_events(NULL, WIFI_APP_EVENTS, WIFI_APP_DISCONNECTED, NULL);
        esp_event_handler_register_with(wifi_app_event_handle,
                                        WIFI_APP_EVENTS,
                                        ESP_EVENT_ANY_ID,
                                        leds_handle_wifi_events,
                                        NULL);
    } else if (usb_app_event_handle) {
        leds_handle_usb_events(NULL, USB_APP_EVENTS, USB_APP_DISCONNECTED, NULL);
        esp_event_handler_register_with(usb_app_event_handle,
                                        USB_APP_EVENTS,
                                        ESP_EVENT_ANY_ID,
                                        leds_handle_usb_events,
                                        NULL);
    }

    if (settings.alert_notification && comm_event_handle) {
        esp_event_handler_register_with(comm_event_handle,
                                        COMM_EVENTS,
                                        ESP_EVENT_ANY_ID,
                                        leds_handle_comm_events,
                                        NULL);
    }

    if (settings.button_notification && button_event_handle) {
        esp_event_handler_register_with(button_event_handle,
                                        BUTTON_EVENTS,
                                        ESP_EVENT_ANY_ID,
                                        leds_handle_btn_events,
                                        NULL);
    }
}

void leds_deinit(void) {
    if (ble_event_handle) {
        esp_event_handler_unregister_with(ble_event_handle,BLE_APP_EVENTS,ESP_EVENT_ANY_ID,leds_handle_ble_events);
    } else if (wifi_app_event_handle) {
        esp_event_handler_unregister_with(wifi_app_event_handle,WIFI_APP_EVENTS,ESP_EVENT_ANY_ID,leds_handle_wifi_events);
    } else if (usb_app_event_handle) {
        esp_event_handler_unregister_with(usb_app_event_handle,USB_APP_EVENTS,ESP_EVENT_ANY_ID,leds_handle_usb_events);
    }

    if (settings.alert_notification && comm_event_handle) {
        esp_event_handler_unregister_with(comm_event_handle,COMM_EVENTS,ESP_EVENT_ANY_ID,leds_handle_comm_events);
    }

    if (settings.button_notification && button_event_handle) {
        esp_event_handler_unregister_with(button_event_handle,BUTTON_EVENTS,ESP_EVENT_ANY_ID,leds_handle_btn_events);
    }
}
