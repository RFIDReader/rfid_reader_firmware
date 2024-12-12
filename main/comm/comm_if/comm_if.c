//
// Created by Pratham Jaiswal on 10/04/24.
//

#include <sys/time.h>
#include "string.h"
#include "esp_log.h"
#include "esp_event.h"

#include "comm_if.h"
#include "settings.h"
#include "silion_sim7200_commands.h"
#include "silion_sim7200.h"
#include "ble_app.h"
#include "tasks_common.h"
#include "battery.h"
#include "utils.h"

static const char TAG[] = "comm_interface";

ESP_EVENT_DEFINE_BASE(COMM_EVENTS);
esp_event_loop_handle_t comm_event_handle;

QueueHandle_t http_msg_queue;
TaskHandle_t merge_task_handle;
QueueHandle_t msg_queue;

void publish(msg_t *msg) {
    switch (settings.comm_mode) {
        case COMM_MODE_BLE:
            ble_app_send_msg(msg);
            break;
        case COMM_MODE_WIFI_AP:
        case COMM_MODE_WIFI_STA:
            if (uxQueueSpacesAvailable(http_msg_queue) < 1) {
                xQueueReset(http_msg_queue);
            }
            xQueueSend(http_msg_queue, msg, pdMS_TO_TICKS(16));
            break;
        case COMM_MODE_USB:
            ESP_LOGE(TAG, "unhandled comm_mode: COMM_MODE_USB");
            break;
        default:
            break;
    }
}

//static void merge_task(void *pvParameters) {
//    while (1) {
//        msg_t msg1, msg2, msg3, msg4;
//        if (xQueueReceive(msg_queue, &msg1, pdMS_TO_TICKS(4)) == pdTRUE) {
//            if (xQueueReceive(msg_queue, &msg2, pdMS_TO_TICKS(4)) == pdTRUE) {
//                msg_t *msg12 = merge_msg(&msg1, &msg2);
//                if (xQueueReceive(msg_queue, &msg3, pdMS_TO_TICKS(4)) == pdTRUE) {
//                    msg_t *msg123 = merge_msg(msg12, &msg3);
//                    if (xQueueReceive(msg_queue, &msg4, pdMS_TO_TICKS(4)) == pdTRUE) {
//                        msg_t *msg1234 = merge_msg(msg123, &msg4);
//                        publish(msg1234);
//                    } else {
//                        publish(msg123);
//                    }
//                } else {
//                    publish(msg12);
//                }
//            } else {
//                publish(&msg1);
//            }
//        }
//        vTaskDelay(pdMS_TO_TICKS(12));
//    }
//}

static void merge_task(void *pvParameters) {
    msg_t msg1, msg2, msg3, msg4;
    msg_t *msgs[4];
    int msg_count = 0;

    while (1) {
        // Wait for the first message indefinitely
        if (xQueueReceive(msg_queue, &msg1, portMAX_DELAY) == pdTRUE) {
            msgs[msg_count++] = &msg1;

            // Try receiving additional messages with a short timeout
            if (xQueueReceive(msg_queue, &msg2, pdMS_TO_TICKS(4)) == pdTRUE) {
                msgs[msg_count++] = &msg2;
            }
            if (xQueueReceive(msg_queue, &msg3, pdMS_TO_TICKS(4)) == pdTRUE) {
                msgs[msg_count++] = &msg3;
            }
            if (xQueueReceive(msg_queue, &msg4, pdMS_TO_TICKS(4)) == pdTRUE) {
                msgs[msg_count++] = &msg4;
            }

            msg_t *merged_msg = merge_msgs(msgs, msg_count);
            if (merged_msg != NULL) {
                publish(merged_msg);
//                free_msg(merged_msg);
            }

            msg_count = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(12));
    }
}

void comm_if_init(void) {
    http_msg_queue = xQueueCreate(32, sizeof(msg_t));
    msg_queue = xQueueCreate(64, sizeof(msg_t));

    esp_event_loop_args_t loop_args = {
            .queue_size = COMM_INTERFACE_TASK_QUEUE_SIZE,
            .task_name = COMM_INTERFACE_TASK_NAME,
            .task_priority = COMM_INTERFACE_TASK_PRIORITY,
            .task_stack_size = COMM_INTERFACE_TASK_STACK_SIZE,
            .task_core_id = COMM_INTERFACE_TASK_CORE_ID,
    };

    esp_event_loop_create(&loop_args, &comm_event_handle);

    xTaskCreatePinnedToCore(
            merge_task,
            "merge_tsk",
            COMM_INTERFACE_TASK_STACK_SIZE,
            NULL,
            uxTaskPriorityGet(NULL),
            &merge_task_handle,
            0);
}

void comm_if_deinit(void) {
    xQueueReset(http_msg_queue);
    xQueueReset(msg_queue);
    esp_event_loop_delete(comm_event_handle);
    vTaskDelete(merge_task_handle);
}

void comm_if_post(msg_t *msg) {
    uint8_t *cmd = msg->data;
    size_t len = msg->len;
    if (len < 3) {
        ESP_LOGE(TAG, "Invalid command detected based on length of cmd.");
        return;
    }
    uint8_t header = cmd[0];
    uint8_t data_len = cmd[1];
    uint8_t code = cmd[2];
    if (header == 0x00) {
        /// this was written for easy debugging
        // printf("received reader command: ");
        // print_hex_arr(cmd, len);
        // printf("\n");
        uint8_t *response_data;
        uint16_t response_len;
        // Reader Command
        switch (code) {
            case 0x00: {
                uint8_t heartbeat_data[] = {0xFF, 0x06, 0xAA, 0x00, 0x00, 0x58, 0x54, 0x53, 0x4A, 0x80, 0x03, 0x17,
                                            0x24, 0xFF, 0x06, 0xAA, 0x00, 0x00, 0x58, 0x54, 0x53, 0x4A, 0x80, 0x03,
                                            0x17, 0x24};
                msg_t heartbeat = {
                        .data = heartbeat_data,
                        .len = 26,
                };
                for (int i = 0; i < 1000; ++i) {
                    comm_if_receive(&heartbeat);
                    vTaskDelay(pdMS_TO_TICKS(1));
                }
            }
            case 0x01: {
                // Comm Mode
                uint8_t val;
                if (data_len != 0x00) {
                    val = cmd[3];
                    settings_set_i8(COMM_MODE, val);
                }
                val = settings_get_i8(COMM_MODE);
                response_len = 6;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = 0x01;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                response_data[5] = (uint8_t) val;
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0x02: {
                // Time
                struct timeval t;
                if (data_len != 0x00) {
                    uint32_t timestamp = 0;
                    timestamp |= ((uint32_t) cmd[3]) << 24;
                    timestamp |= ((uint32_t) cmd[4]) << 16;
                    timestamp |= ((uint32_t) cmd[5]) << 8;
                    timestamp |= (uint32_t) cmd[6];
                    t.tv_sec = timestamp;
                    settimeofday(&t, NULL);
                }
                gettimeofday(&t, NULL);
                uint8_t val_len = 4;
                response_len = 5 + val_len;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = val_len;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                for (int i = 0; i < val_len; ++i) {
                    response_data[5 + i] = (uint8_t) (t.tv_sec >> ((3 - i) * 8));
                }
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0x03: {
                // Network STA SSID
                char *val;
                if (data_len != 0x00) {
                    val = (char *) malloc((data_len + 1) * sizeof(char));
                    for (uint8_t i = 0; i < data_len; ++i) {
                        val[i] = (char) cmd[3 + i];
                    }
                    val[data_len] = '\0';
                    settings_set_str(NETWORK_SETUP_WIFI_STA_SSID, val);
                }
                val = settings_get_str(NETWORK_SETUP_WIFI_STA_SSID);
                uint8_t val_len = strlen(val);
                response_len = 5 + val_len;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = val_len;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                for (uint8_t i = 0; i < val_len; ++i) {
                    response_data[5 + i] = val[i];
                }
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0x04: {
                // Network STA PSWD
                char *val;
                if (data_len != 0x00) {
                    val = (char *) malloc((data_len + 1) * sizeof(char));
                    for (uint8_t i = 0; i < data_len; ++i) {
                        val[i] = (char) cmd[3 + i];
                    }
                    val[data_len] = '\0';
                    settings_set_str(NETWORK_SETUP_WIFI_STA_PSWD, val);
                }
                val = settings_get_str(NETWORK_SETUP_WIFI_STA_PSWD);
                uint8_t val_len = strlen(val);
                response_len = 5 + val_len;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = val_len;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                for (uint8_t i = 0; i < val_len; ++i) {
                    response_data[5 + i] = val[i];
                }
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0x05: {
                // Button notification
                uint8_t val;
                if (data_len != 0x00) {
                    val = cmd[3];
                    settings_set_i8(BTN_NOTIFICATION, val);
                }
                val = settings_get_i8(BTN_NOTIFICATION);
                response_len = 6;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = 0x01;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                response_data[5] = (uint8_t) val;
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0x06: {
                // Alert notification
                uint8_t val;
                if (data_len != 0x00) {
                    val = cmd[3];
                    settings_set_i8(ALERT_NOTIFICATION, val);
                }
                val = settings_get_i8(ALERT_NOTIFICATION);
                response_len = 6;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = 0x01;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                response_data[5] = (uint8_t) val;
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0x07: {
                // System display mode
                uint8_t val;
                if (data_len != 0x00) {
                    val = cmd[3];
                    settings_set_i8(SYSTEM_DISPLAY_MODE, val);
                }
                val = settings_get_i8(SYSTEM_DISPLAY_MODE);
                response_len = 6;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = 0x01;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                response_data[5] = (uint8_t) val;
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0x08: {
                // Device name
                char *val;
                if (data_len != 0x00) {
                    val = (char *) malloc((data_len + 1) * sizeof(char));
                    for (uint8_t i = 0; i < data_len; ++i) {
                        val[i] = (char) cmd[3 + i];
                    }
                    val[data_len] = '\0';
                    settings_set_str(SYSTEM_DEVICE_NAME, val);
                }
                val = settings_get_str(SYSTEM_DEVICE_NAME);
                uint8_t val_len = strlen(val);
                response_len = 5 + val_len;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = val_len;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                for (uint8_t i = 0; i < val_len; ++i) {
                    response_data[5 + i] = val[i];
                }
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0xB1: {
                uint8_t val;
                if (data_len != 0x00) {
                    for (uint8_t i = 0; i < data_len; ++i) {
                        val = cmd[3 + i];
                    }
                    settings_set_i8(SYSTEM_BTN_CLICK_ACTION, val);
                }
                val = settings_get_i8(SYSTEM_BTN_CLICK_ACTION);
                response_len = 6;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = 0x01;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                response_data[5] = (uint8_t) val;
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0xB2: {
                uint8_t *val;
                if (data_len != 0) {
                    val = (uint8_t *) malloc((data_len) * sizeof(uint8_t));
                    for (uint8_t i = 0; i < data_len; ++i) {
                        val[i] = cmd[3 + i];
                    }
                    settings_set_blob(SYSTEM_BTN_PRESS_ACTION, val, data_len);
                }
                val = settings_get_blob(SYSTEM_BTN_PRESS_ACTION);
                uint8_t val_len = val[0];
                response_len = 5 + val_len;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = val_len;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                for (uint8_t i = 0; i < val_len; ++i) {
                    response_data[5 + i] = val[i + 1];
                }
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0xB3: {
                uint8_t *val;
                if (data_len != 0) {
                    val = (uint8_t *) malloc((data_len) * sizeof(uint8_t));
                    for (uint8_t i = 0; i < data_len; ++i) {
                        val[i] = cmd[3 + i];
                    }
                    settings_set_blob(SYSTEM_BTN_RELEASE_ACTION, val, data_len);
                }
                val = settings_get_blob(SYSTEM_BTN_RELEASE_ACTION);
                uint8_t val_len = val[0];
                response_len = 5 + val_len;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = val_len;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                for (uint8_t i = 0; i < val_len; ++i) {
                    response_data[5 + i] = val[i + 1];
                }
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0xBA: {
                uint8_t val = (uint8_t) battery_state.soc;
                uint8_t val_len = 1;
                response_len = 5 + val_len;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = val_len;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                response_data[5] = val;
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0xBB: {
                uint8_t val = (uint8_t) battery_state.soh;
                uint8_t val_len = 1;
                response_len = 5 + val_len;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = val_len;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                response_data[5] = val;
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0xBC: {
                uint8_t val = (uint8_t) battery_state.cycle_count;
                uint8_t val_len = 1;
                response_len = 5 + val_len;
                response_data = (uint8_t *) malloc(response_len * sizeof(uint8_t));
                response_data[0] = header;
                response_data[1] = val_len;
                response_data[2] = code;
                response_data[3] = 0x00;
                response_data[4] = 0x00;
                response_data[5] = val;
                msg_t response = {response_data, response_len};
                comm_if_receive(&response);
                break;
            }
            case 0xFD: {
                settings_reset();
                uint8_t factory_reset_data[] = {0xFF, 0x10, 0xAA, 0x4D, 0x6F, 0x64, 0x75, 0x6C, 0x65, 0x74, 0x65, 0x63,
                                                0x68, 0xAA, 0x40, 0xAA, 0x01, 0x95, 0xBB, 0x8D, 0x63};
                msg_t module_factory_reset = {
                        .data = factory_reset_data,
                        .len = 21
                };
                comm_if_post(&module_factory_reset);
                esp_restart();
            }
            case 0xFE: {
                esp_restart();
            }
            case 0xFF: {

            }
            default:
                ESP_LOGE(TAG, "unknown command code %02X", code);
        }
    } else if (header == 0xFF) {
        // RFID Command
        bool is_known_cmd = false;
        for (int i = 0; i < SILION_COMMANDS_COUNT; ++i) {
            if (SILION_COMMANDS[i] == code) {
                is_known_cmd = true;
                ESP_LOGD(TAG, "received command is silion_sim7200 command");
                break;
            }
        }
        if (is_known_cmd) {
            silion_sim7200_send(msg);
        } else {
            ESP_LOGE(TAG, "unknown command code %02X", code);
        }
    }
}

void comm_if_receive(msg_t *msg) {
    /// DATA_RECEIVED: Currently not useful.
    /// SCAN_STARTED / SCAN_STOPPED: Useful for updating the display's status, and to start / stop fans
    /// TAG_READ: Useful for displaying tag values on the screen when not connected to a host device, and for triggering user-configured alerts (such as beeps or LED indications).
    /// HEARTBEAT_RECEIVED: Useful for displaying a heartbeat to inform the user that scanning is ongoing, especially when no tags are present in the environment.
    /// ✅ ENCOUNTERED_ERROR: Useful for displaying status code

    if (xQueueSend(msg_queue, msg, pdMS_TO_TICKS(2)) == errQUEUE_FULL) {
        publish(msg);
    }
}