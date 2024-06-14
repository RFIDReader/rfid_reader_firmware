//
// Created by Pratham Jaiswal on 10/04/24.
//

#ifndef RFID_READER_COMM_IF_H
#define RFID_READER_COMM_IF_H

#include "freertos/queue.h"
#include "esp_event.h"

#include "types.h"

ESP_EVENT_DECLARE_BASE(COMM_EVENTS);

enum {
    COMM_EVENT_DATA_RECEIVED,
    COMM_EVENT_SCAN_STARTED,
    COMM_EVENT_SCAN_STOPPED,
    COMM_EVENT_TAG_READ,
    COMM_EVENT_HEARTBEAT_RECEIVED,
    COMM_EVENT_ENCOUNTERED_ERROR
};

extern esp_event_loop_handle_t comm_event_handle;

/**
 * msg_t queue for http communication
 * REQUEST
 *    |----> comm_if_post -> process -> comm_if_receive -> xQueueSend
 *    |----> xQueueReceive -> RESPONSE
 */
extern QueueHandle_t http_msg_queue;

void comm_if_init(void);

void comm_if_post(msg_t *msg);

void comm_if_receive(msg_t *msg);

#endif //RFID_READER_COMM_IF_H
