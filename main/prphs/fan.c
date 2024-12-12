//
// Created by Pratham Jaiswal on 20/03/24.
//

#include "fan.h"

#ifdef FAN_SUPPORTED

#include "tasks_common.h"
#include "comm_if.h"

static bool fan_state = 0;

static unsigned int _on_time_ms;
static unsigned int _off_time_ms;

static void fan_task(void *pvParameters) {
    while (fan_state) {
        gpio_set_level(FAN_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(_on_time_ms));
        gpio_set_level(FAN_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(_off_time_ms));
    }

    vTaskDelete(NULL);
}

void fan_on(unsigned int on_time_ms, unsigned int off_time_ms) {
    _on_time_ms = on_time_ms;
    _off_time_ms = off_time_ms;
    fan_state = 1;
    xTaskCreatePinnedToCore(fan_task,
                            "fan_tsk",
                            TEMP_TASK_STACK_SIZE,
                            NULL,
                            TEMP_TASK_PRIORITY,
                            NULL,
                            TEMP_TASK_CORE_ID);
}

void fan_off(void) {
    fan_state = 0;
}

void fan_handle_comm_events(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    switch (event_id) {
        case COMM_EVENT_SCAN_STARTED:
            fan_on(5000, 1000);
            break;
        case COMM_EVENT_SCAN_STOPPED:
            fan_off();
            break;
        default:
            break;
    }
}

#endif

void fan_init(void) {
#ifdef FAN_SUPPORTED
    gpio_set_direction(FAN_GPIO, GPIO_MODE_OUTPUT);
    esp_event_handler_register_with(comm_event_handle, COMM_EVENTS, ESP_EVENT_ANY_ID, fan_handle_comm_events, NULL);
#endif
}

void fan_deinit(void) {
#ifdef FAN_SUPPORTED
    esp_event_handler_unregister_with(comm_event_handle, COMM_EVENTS, ESP_EVENT_ANY_ID, fan_handle_comm_events);
#endif
}
