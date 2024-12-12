//
// Created by Pratham Jaiswal on 20/03/24.
//

#include <freertos/FreeRTOS.h>
#include <esp_event.h>
#include "buzzer.h"
#include "ble_app.h"
#include "settings.h"
#include "app_state_disp/app_state_disp.h"
#include "buttons.h"
#include "tasks_common.h"
#include "comm_if.h"
#include "wifi_app.h"
#include "usb_app.h"

void buzzer_handle_connected() {
    buzzer_on(180);
    vTaskDelay(pdMS_TO_TICKS(270));
    buzzer_on(360);
}

void buzzer_handle_disconnected() {
    buzzer_on(360);
    vTaskDelay(pdMS_TO_TICKS(450));
    buzzer_on(180);
}

void buzzer_handle_ble_events(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    event_id == BLE_EVENT_CONNECTED ? buzzer_handle_connected() : buzzer_handle_disconnected();
}

void buzzer_handle_wifi_events(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    event_id == WIFI_APP_CONNECTED ? buzzer_handle_connected() : buzzer_handle_disconnected();
}

void buzzer_handle_usb_events(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    event_id == USB_APP_CONNECTED ? buzzer_handle_connected() : buzzer_handle_disconnected();
}

void beep(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data) {
    buzzer_on(50);
}

void buzzer_init(void) {
    gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);
    esp_event_handler_register_with(app_state_event_handle,
                                    APP_STATE_EVENTS,
                                    APP_STATE_OPERATIONAL,
                                    beep,
                                    NULL);

    if (ble_event_handle) {
        esp_event_handler_register_with(ble_event_handle,
                                        BLE_APP_EVENTS,
                                        ESP_EVENT_ANY_ID,
                                        buzzer_handle_ble_events,
                                        NULL);
    } else if (wifi_app_event_handle) {
        esp_event_handler_register_with(wifi_app_event_handle,
                                        WIFI_APP_EVENTS,
                                        ESP_EVENT_ANY_ID,
                                        buzzer_handle_wifi_events,
                                        NULL);
    } else if (usb_app_event_handle) {
        esp_event_handler_register_with(usb_app_event_handle,
                                        USB_APP_EVENTS,
                                        ESP_EVENT_ANY_ID,
                                        buzzer_handle_usb_events,
                                        NULL);
    }

    if (settings.alert_notification && comm_event_handle) {
        esp_event_handler_register_with(comm_event_handle,
                                        COMM_EVENTS,
                                        ESP_EVENT_ANY_ID,
                                        beep,
                                        NULL);
    }

    if (settings.button_notification && button_event_handle) {
        esp_event_handler_register_with(button_event_handle,
                                        BUTTON_EVENTS,
                                        ESP_EVENT_ANY_ID,
                                        beep,
                                        NULL);
    }
}

void buzzer_deinit() {
    esp_event_handler_unregister_with(app_state_event_handle, APP_STATE_EVENTS, APP_STATE_OPERATIONAL, beep);
    if (ble_event_handle) {
        esp_event_handler_unregister_with(ble_event_handle, BLE_APP_EVENTS, ESP_EVENT_ANY_ID, buzzer_handle_ble_events);
    } else if (wifi_app_event_handle) {
        esp_event_handler_unregister_with(wifi_app_event_handle, WIFI_APP_EVENTS, ESP_EVENT_ANY_ID,
                                          buzzer_handle_wifi_events);
    } else if (usb_app_event_handle) {
        esp_event_handler_unregister_with(usb_app_event_handle, USB_APP_EVENTS, ESP_EVENT_ANY_ID,
                                          buzzer_handle_usb_events);
    }

    if (settings.alert_notification && comm_event_handle)
        esp_event_handler_unregister_with(comm_event_handle, COMM_EVENTS, ESP_EVENT_ANY_ID, beep);
    if (settings.button_notification && button_event_handle)
        esp_event_handler_unregister_with(button_event_handle, BUTTON_EVENTS, ESP_EVENT_ANY_ID, beep);
}

static void buzzer_task(void *pvParameter) {
    uint32_t *duration_ms = (uint32_t *) pvParameter;
    gpio_set_level(BUZZER_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(*duration_ms));
    gpio_set_level(BUZZER_GPIO, 0);
    vTaskDelete(NULL);
}

void buzzer_on(uint32_t duration_ms) {
    uint32_t *ms = (uint32_t *) malloc(sizeof(uint32_t));
    *ms = duration_ms;
    xTaskCreatePinnedToCore(buzzer_task,
                            "buzzer_tsk",
                            TEMP_TASK_STACK_SIZE,
                            (void *) ms,
                            TEMP_TASK_PRIORITY,
                            NULL,
                            TEMP_TASK_CORE_ID);
}

//void play_tone(uint16_t frequency, uint32_t duration_ms) {
//    if (frequency == 0) {
//        vTaskDelay(pdMS_TO_TICKS(duration_ms));
//        return;
//    }
//
//    uint32_t half_period_us = (uint32_t) (1000000 / (2 * frequency));
//    uint32_t cycles = (uint32_t) (duration_ms * 1000 / (2 * half_period_us));
//
//    for (uint32_t i = 0; i < cycles; i++) {
//        gpio_set_level(BUZZER_GPIO, 1);
//        esp_rom_delay_us(half_period_us);
//        gpio_set_level(BUZZER_GPIO, 0);
//        esp_rom_delay_us(half_period_us);
//    }
//}
//
//void buzzer_play_melody(uint16_t *notes, uint32_t *durations, uint16_t length, uint8_t slow) {
//    for (uint32_t i = 0; i < length; i++) {
//        play_tone(notes[i], durations[i]);
//        vTaskDelay(pdMS_TO_TICKS(slow));
//    }
//}
