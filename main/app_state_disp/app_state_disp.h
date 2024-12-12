//
// Created by Pratham Jaiswal on 10/04/24.
//

#ifndef RFID_READER_APP_STATE_DISPATCHER_H
#define RFID_READER_APP_STATE_DISPATCHER_H

#include "esp_event.h"

#define STARTUP_WAIT_TIME_MS 3000

ESP_EVENT_DECLARE_BASE(APP_STATE_EVENTS);

typedef enum {
    APP_STATE_STARTUP,
    APP_STATE_CONFIGURATION,
    APP_STATE_INITIALIZATION,
    APP_STATE_OPERATIONAL,
    APP_STATE_IDLE_WARNING,
    APP_STATE_DEINITIALIZATION,
    APP_STATE_SLEEPING,
} app_state_t;

extern bool comm_idle;
extern bool buttons_idle;
extern bool rfid_idle;

extern app_state_t app_state;
extern esp_event_loop_handle_t app_state_event_handle;

void app_state_init(void),
        app_state_dispatch_init_state(void),
        app_state_dispatch_op_state(void),
        app_state_set_comm_idle_status(bool),
        app_state_set_buttons_idle_status(bool),
        app_state_set_rfid_idle_status(bool);

#endif //RFID_READER_APP_STATE_DISPATCHER_H
