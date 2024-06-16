//
// Created by Pratham Jaiswal on 20/03/24.
//

#include <esp_log.h>
#include <string.h>

#include "silion_sim7200.h"
#include "silion_sim7200_status_codes.h"
#include "comm_if.h"
#include "tasks_common.h"
#include "utils.h"

void uart_comm_init(void);

void uart_comm_write(msg_t *msg);

static const char TAG[] = "silion_sim7200";

ESP_EVENT_DEFINE_BASE(SILION_SIM7200_EVENTS);
esp_event_loop_handle_t silion_sim7200_event_handle;

static QueueHandle_t uart2_queue;

static const int BUF_SIZE = 512;

void silion_sim7200_send(msg_t *msg) {
    ESP_LOGD(TAG, "silion_sim7200_post");
    // format to silion command
    /// header (1byte) (FF) - data size (1byte) - command code (1byte) - data (n bytes) - crc16 (2bytes)
    uart_comm_write(msg);
}

void silion_sim7200_receive(msg_t *msg) {
    // format of silion response
    /// header (1byte) (FF) - data size (1byte) - command code (1byte) - status code (2bytes) - data (n bytes) - crc16 (2bytes)
    // so if message is received: header, data size, command code, status code, crc16 is minimum expected which is 7 bytes
    comm_if_receive(msg);
}

void silion_sim7200_init() {
    uart_comm_init();

    esp_event_loop_args_t loop_args = {
            .queue_size = RFID_MODULE_TASK_QUEUE_SIZE,
            .task_name = RFID_MODULE_TASK_NAME,
            .task_priority = RFID_MODULE_TASK_PRIORITY,
            .task_stack_size = RFID_MODULE_TASK_STACK_SIZE,
            .task_core_id = RFID_MODULE_TASK_CORE_ID,
    };

    esp_event_loop_create(&loop_args, &silion_sim7200_event_handle);
}

static void rx_task(void *pvParameters) {
    uart_event_t event;
    size_t buffered_size;
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    while (xQueueReceive(uart2_queue, (void *) &event, (TickType_t) portMAX_DELAY)) {
        bzero(data, BUF_SIZE);
        switch (event.type) {
            case UART_DATA:
                uart_read_bytes(UART, data, event.size, portMAX_DELAY);
                msg_t msg = {data, event.size};
                silion_sim7200_receive(&msg);
                break;
            case UART_FIFO_OVF:
                ESP_LOGW(TAG, "hw fifo overflow");
                uart_flush_input(UART);
                xQueueReset(uart2_queue);
                break;
            case UART_BUFFER_FULL:
                ESP_LOGW(TAG, "ring buffer full");
                uart_flush_input(UART);
                xQueueReset(uart2_queue);
                break;
            case UART_BREAK:
                ESP_LOGE(TAG, "uart rx break");
                break;
            case UART_PARITY_ERR:
                ESP_LOGE(TAG, "uart parity error");
                break;
            case UART_FRAME_ERR:
                ESP_LOGE(TAG, "uart frame error");
                break;
            case UART_PATTERN_DET:
                uart_get_buffered_data_len(UART, &buffered_size);
                int pos = uart_pattern_pop_pos(UART);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1) {
                    uart_flush_input(UART);
                } else {
                    uart_read_bytes(UART, data, pos, pdMS_TO_TICKS(100));
                    uint8_t pat[PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(UART, pat, PATTERN_CHR_NUM, pdMS_TO_TICKS(100));
                    ESP_LOGI(TAG, "read data: %s", data);
                    ESP_LOGI(TAG, "read pat : %s", pat);
                }
                break;
            default:
                ESP_LOGW(TAG, "uart event type: %d", event.type);
                break;
        }
    }

    free(data);
    data = NULL;
    vTaskDelete(NULL);
}

void uart_comm_init(void) {
    uart_driver_install(UART, BUF_SIZE * 2, 0, 128, &uart2_queue, 0);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_enable_pattern_det_baud_intr(UART, '+', PATTERN_CHR_NUM, 9, 0, 0);
    uart_pattern_queue_reset(UART, 20);

    // boot firmware
    uint8_t boot_firmware_data[] = {0xFF, 0x00, 0x04, 0x1D, 0x0B};
    msg_t boot_firmware = {
            .data = boot_firmware_data,
            .len = 5,
    };
    uart_comm_write(&boot_firmware);

    xTaskCreatePinnedToCore(rx_task,
                            "rx_tsk",
                            UART_COMM_TASK_STACK_SIZE,
                            NULL,
                            UART_COMM_TASK_PRIORITY,
                            NULL,
                            UART_COMM_TASK_CORE_ID);
}

void uart_comm_write(msg_t *msg) {
    const int txBytes = uart_write_bytes(UART, msg->data, msg->len);
    if (txBytes == msg->len) {
        ESP_LOGD(TAG, "write successful");
    }
}