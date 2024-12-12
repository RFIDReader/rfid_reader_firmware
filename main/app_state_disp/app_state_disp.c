//
// Created by Pratham Jaiswal on 10/04/24.
//

#include <esp_log.h>
#include <freertos/idf_additions.h>

#include "app_state_disp.h"
#include "buttons.h"
#include "tasks_common.h"

static const char TAG[] = "app_state_dispatcher";

ESP_EVENT_DEFINE_BASE(APP_STATE_EVENTS);

bool comm_idle = false;
bool buttons_idle = false;
bool rfid_idle = false;
bool idle = false;
TimerHandle_t idle_timer;

app_state_t app_state;
esp_event_loop_handle_t app_state_event_handle;

void dispatch(app_state_t new_app_state) {
    ESP_LOGI(TAG, "app_state change %d -> %d", app_state, new_app_state);
    app_state = new_app_state;

    esp_event_post_to(app_state_event_handle,
                      APP_STATE_EVENTS,
                      app_state,
                      NULL,
                      0,
                      portMAX_DELAY);
}

void app_state_button_handler(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    ESP_LOGI(TAG, "app_state_button_handler event_id -> C_BUTTON_PRESSED");
    if (app_state == APP_STATE_STARTUP) {
        ESP_LOGI(TAG, "C_BUTTON_PRESSED entering boot_config");
        dispatch(APP_STATE_CONFIGURATION);

        ESP_LOGI(TAG, "unregistering button event handler");
        esp_event_handler_unregister_with(button_event_handle,
                                          BUTTON_EVENTS,
                                          C_BUTTON_PRESSED,
                                          app_state_button_handler);
    }
}

void boot_task(void *pvParameters) {
    ESP_LOGI(TAG, "boot_task waiting STARTUP_WAIT_TIME_MS before switching to APP_STATE_INITIALIZATION");
    vTaskDelay(pdMS_TO_TICKS(STARTUP_WAIT_TIME_MS));
    if (app_state == APP_STATE_STARTUP) {
        app_state_dispatch_init_state();
    }
    vTaskDelete(NULL);
}

void app_state_init() {
    ESP_LOGI(TAG, "app_state_init is running on Core %d\n", xPortGetCoreID());

    esp_event_loop_args_t loop_args = {
            .queue_size = APP_STATE_DISP_TASK_QUEUE_SIZE,
            .task_name = APP_STATE_DISP_TASK_NAME,
            .task_priority = APP_STATE_DISP_TASK_PRIORITY,
            .task_stack_size = APP_STATE_DISP_TASK_STACK_SIZE,
            .task_core_id = APP_STATE_DISP_TASK_CORE_ID,
    };

    esp_event_loop_create(&loop_args, &app_state_event_handle);

    vTaskDelay(pdMS_TO_TICKS(5));
    dispatch(APP_STATE_STARTUP);

    esp_event_handler_register_with(button_event_handle,
                                    BUTTON_EVENTS,
                                    C_BUTTON_PRESSED,
                                    app_state_button_handler,
                                    NULL);

    xTaskCreatePinnedToCore(boot_task,
                            "boot_tsk",
                            TEMP_TASK_STACK_SIZE,
                            NULL,
                            TEMP_TASK_PRIORITY,
                            NULL,
                            TEMP_TASK_CORE_ID);
}

void app_state_dispatch_init_state() {
    ESP_LOGI(TAG, "app_state_enter_init_state");
    if (app_state == APP_STATE_STARTUP || app_state == APP_STATE_CONFIGURATION)
        dispatch(APP_STATE_INITIALIZATION);
    else
        ESP_LOGE(TAG, "cannot enter INITIALIZATION state, current state is %d", app_state);
}

void app_state_dispatch_op_state() {
    ESP_LOGI(TAG, "app_state_enter_op_state");
    if (app_state == APP_STATE_INITIALIZATION)
        dispatch(APP_STATE_OPERATIONAL);
    else
        ESP_LOGE(TAG, "cannot enter OPERATIONAL state, current state is %d", app_state);
}

void app_state_dispatch_idle_state() {
    ESP_LOGI(TAG, "app_state_enter_idle_state");
    dispatch(APP_STATE_IDLE_WARNING);
}

void app_state_dispatch_deinit_state() {
    ESP_LOGI(TAG, "app_state_enter_deinit_state");
    dispatch(APP_STATE_DEINITIALIZATION);
}

void app_state_dispatch_sleeping_state() {
    ESP_LOGI(TAG, "app_state_enter_sleeping_state");
    dispatch(APP_STATE_SLEEPING);
}

void tmr_dispatch_idle_state(TimerHandle_t xTimer) {
    ESP_LOGI(TAG, "tmr_dispatch_idle_state");
    app_state_dispatch_idle_state();
}

void app_state_update_global_idle_status(void) {
    ESP_LOGI(TAG, "app_state_update_global_idle_status");
    idle = comm_idle & buttons_idle & rfid_idle;
    if (idle) {
        ESP_LOGI(TAG, "idle: setting timer to enter idle state");
        idle_timer = xTimerCreate(
                "idle_tmr",
                pdMS_TO_TICKS(10000),
                pdFALSE,
                (void *) 0,
                tmr_dispatch_idle_state);
    } else {
        ESP_LOGI(TAG, "not idle");
        if(xTimerIsTimerActive(idle_timer)) {
            ESP_LOGI(TAG, "deleting timer to enter idle state");
            xTimerDelete(idle_timer, pdMS_TO_TICKS(1));
        }
    }
}

void app_state_set_comm_idle_status(bool status) {
    ESP_LOGI(TAG, "app_state_set_comm_idle_status");
    comm_idle = status;
    app_state_update_global_idle_status();
}

void app_state_set_buttons_idle_status(bool status) {
    ESP_LOGI(TAG, "app_state_set_buttons_idle_status");
    buttons_idle = status;
    app_state_update_global_idle_status();
}

void app_state_set_rfid_idle_status(bool status) {
    ESP_LOGI(TAG, "app_state_set_rfid_idle_status");
    rfid_idle = status;
    app_state_update_global_idle_status();
}
