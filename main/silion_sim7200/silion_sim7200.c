//
// Created by Pratham Jaiswal on 20/03/24.
//

#include <esp_log.h>
#include <string.h>

#include "silion_sim7200.h"
#include "silion_sim7200_status_codes.h"
#include "comm_if.h"
#include "tasks_common.h"

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
    if (uart_is_driver_installed(UART)) {
        uart_driver_delete(UART);
    }
    uart_driver_install(UART, BUF_SIZE * 2, 0, 128, &uart2_queue, 0);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_enable_pattern_det_baud_intr(UART, '+', PATTERN_CHR_NUM, 9, 0, 0);
    uart_pattern_queue_reset(UART, 20);

    uint8_t data[] = {0xFF, 0x00, 0x0C, 0x1D, 0x03}; // Get Reader Runtime
    size_t data_len = 5;
    uart_write_bytes(UART, data, data_len);

    uint8_t buf[8] = {0};
    int ret = uart_read_bytes(UART, buf, 8, pdMS_TO_TICKS(33));
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to read data from UART");
    }
    uint8_t expect_data[5] = {0xFF, 0x01, 0x0C, 0x00, 0x00};
    bool result = true;
    for (int i = 0; i < 5; ++i) {
        if (buf[i] != expect_data[i]) {
            result = false;
            break;
        }
    }

    if (result) { // received data matched expected data
        ESP_LOGI(TAG, "Reader connected at baud rate %d", uart_config.baud_rate);
        if (uart_config.baud_rate == 115200) {
            uint8_t cmd[] = {0xFF, 0x14, 0xAA, 0x4D, 0x6F, 0x64, 0x75, 0x6C, 0x65, 0x74, 0x65, 0x63, 0x68, 0xAA, 0x40,
                             0x06, 0x01, 0x00, 0x0E, 0x10, 0x00, 0x0F, 0xBB, 0x79, 0x9F};
            size_t cmd_len = 25;
            uart_write_bytes(UART, cmd, cmd_len);
        }
        // boot firmware
        uint8_t boot_firmware[] = {0xFF, 0x00, 0x04, 0x1D, 0x0B};
        uint8_t boot_firmware_len = 5;
        uart_write_bytes(UART, boot_firmware, boot_firmware_len);
    } else {
        ESP_LOGE(TAG, "Reader did not connect at baud rate %d\nAttempting to connect using baud rate 115200",
                 uart_config.baud_rate);
        if (uart_config.baud_rate == 921600) {
            uart_config.baud_rate = 115200;
            return uart_comm_init();
        } else { // uart_config.baud_rate is 115200
            ESP_LOGE(TAG, "Reader did not connect at baud rate 115200");
            return;
        }
    }

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