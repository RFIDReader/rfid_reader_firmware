//
// Created by Pratham Jaiswal on 20/03/24.
//

#include <esp_log.h>
#include "iot_button.h"

#include "buttons.h"

static const char TAG[] = "BUTTON";

ESP_EVENT_DEFINE_BASE(BUTTON_EVENTS);

esp_event_loop_handle_t button_event_handle;

button_handle_t scan_btn_handle;
button_handle_t prev_btn_handle;
button_handle_t next_btn_handle;
button_handle_t c_btn_handle;
button_handle_t ok_btn_handle;

button_config_t scan_btn = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
                .gpio_num = SCAN_BTN_GPIO,
                .active_level = 0,
        }
};

button_config_t prev_btn = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {PREV_BTN_GPIO, 0}
};

button_config_t next_btn = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {NEXT_BTN_GPIO, 0}
};

button_config_t c_btn = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {C_BTN_GPIO, 0}
};

button_config_t ok_btn = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {OK_BTN_GPIO, 0}
};

/// capture pressed and released event
static void scan_btn_cb(void *arg, void *user_data) {
    int evt = (button_event_t) user_data;
    ESP_LOGI(TAG, "scan button %d", evt);
    esp_event_post_to(button_event_handle,
                      BUTTON_EVENTS,
                      evt,
                      NULL,
                      0,
                      pdMS_TO_TICKS(16));
}

void prev_btn_cb(void *arg, void *user_data) {
    int evt = (button_event_t) user_data;
    ESP_LOGI(TAG, "prev button %d", evt);
    esp_event_post_to(button_event_handle,
                      BUTTON_EVENTS,
                      evt,
                      NULL,
                      0,
                      pdMS_TO_TICKS(16));
}

void next_btn_cb(void *arg, void *user_data) {
    int evt = (button_event_t) user_data;
    ESP_LOGI(TAG, "next button %d", evt);
    esp_event_post_to(button_event_handle,
                      BUTTON_EVENTS,
                      evt,
                      NULL,
                      0,
                      pdMS_TO_TICKS(16));
}

void c_btn_cb(void *arg, void *user_data) {
    int evt = (button_event_t) user_data;
    ESP_LOGI(TAG, "c button %d", evt);
    esp_event_post_to(button_event_handle,
                      BUTTON_EVENTS,
                      evt,
                      NULL,
                      0,
                      pdMS_TO_TICKS(16));
}

/// capture single click and long press event
void ok_btn_cb(void *arg, void *user_data) {
    int evt = (button_event_t) user_data;
    ESP_LOGI(TAG, "ok button %d", evt);
    esp_event_post_to(button_event_handle,
                      BUTTON_EVENTS,
                      evt,
                      NULL,
                      0,
                      pdMS_TO_TICKS(16));
}

void buttons_init(void) {
    esp_event_loop_args_t loop_args = {
            .queue_size = 10,
            .task_name = "btn_loop",
            .task_priority = uxTaskPriorityGet(NULL),
            .task_stack_size = 4096,
            .task_core_id = 0,
    };

    esp_event_loop_create(&loop_args, &button_event_handle);

    scan_btn_handle = iot_button_create(&scan_btn);
    if (scan_btn_handle == NULL) {
        ESP_LOGE(TAG, "button create failed: SCAN_BTN_GPIO");
    }

    prev_btn_handle = iot_button_create(&prev_btn);
    if (prev_btn_handle == NULL) {
        ESP_LOGE(TAG, "button create failed: PREV_BTN_GPIO");
    }

    next_btn_handle = iot_button_create(&next_btn);
    if (next_btn_handle == NULL) {
        ESP_LOGE(TAG, "button create failed: NEXT_BTN_GPIO");
    }

    c_btn_handle = iot_button_create(&c_btn);
    if (c_btn_handle == NULL) {
        ESP_LOGE(TAG, "button create failed: C_BTN_GPIO");
    }

    ok_btn_handle = iot_button_create(&ok_btn);
    if (ok_btn_handle == NULL) {
        ESP_LOGE(TAG, "button create failed: OK_BTN_GPIO");
    }

    iot_button_register_cb(scan_btn_handle, BUTTON_SINGLE_CLICK, scan_btn_cb, (void *) SCAN_BUTTON_PRESSED);
    iot_button_register_cb(scan_btn_handle, BUTTON_LONG_PRESS_START, scan_btn_cb, (void *) SCAN_BUTTON_HOLD);
    iot_button_register_cb(scan_btn_handle, BUTTON_LONG_PRESS_UP, scan_btn_cb, (void *) SCAN_BUTTON_RELEASED);
    iot_button_register_cb(prev_btn_handle, BUTTON_SINGLE_CLICK, prev_btn_cb, (void *) PREV_BUTTON_PRESSED);
    iot_button_register_cb(next_btn_handle, BUTTON_SINGLE_CLICK, next_btn_cb, (void *) NEXT_BUTTON_PRESSED);
    iot_button_register_cb(c_btn_handle, BUTTON_SINGLE_CLICK, c_btn_cb, (void *) C_BUTTON_PRESSED);
    iot_button_register_cb(ok_btn_handle, BUTTON_SINGLE_CLICK, ok_btn_cb, (void *) OK_BUTTON_PRESSED);
    iot_button_register_cb(ok_btn_handle, BUTTON_LONG_PRESS_START, ok_btn_cb, (void *) OK_BUTTON_HOLD);
}

void buttons_deinit(void) {
    iot_button_unregister_cb(scan_btn_handle, BUTTON_SINGLE_CLICK);
    iot_button_unregister_cb(scan_btn_handle, BUTTON_LONG_PRESS_START);
    iot_button_unregister_cb(scan_btn_handle, BUTTON_LONG_PRESS_UP);
    iot_button_unregister_cb(prev_btn_handle, BUTTON_SINGLE_CLICK);
    iot_button_unregister_cb(next_btn_handle, BUTTON_SINGLE_CLICK);
    iot_button_unregister_cb(c_btn_handle, BUTTON_SINGLE_CLICK);
    iot_button_unregister_cb(ok_btn_handle, BUTTON_SINGLE_CLICK);
    iot_button_unregister_cb(ok_btn_handle, BUTTON_LONG_PRESS_START);

    iot_button_delete(scan_btn_handle);
    iot_button_delete(prev_btn_handle);
    iot_button_delete(next_btn_handle);
    iot_button_delete(c_btn_handle);
    iot_button_delete(ok_btn_handle);
}