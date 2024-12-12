//
// Created by Pratham Jaiswal on 16/06/24.
//

#include <string.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart.h"
#include "types.h"
#include "tasks_common.h"
#include "silion_sim7200.h"

static const char TAG[] = "uart";

TaskHandle_t uart_task_handle;
static QueueHandle_t uart2_queue;

static const int BUF_SIZE = 512;

static void rx_task(void *pvParameters);

bool test_uart_comm(void);

int uart_detect_baud_rate(void);

bool uart_check_baud_rate(int baud_rate);

void uart_init(void) {
    int baud_rate = uart_detect_baud_rate();
    if (baud_rate == -1) {
        ESP_LOGE(TAG, "Failed to detect baud rate");
    }

    uart_driver_install(UART, BUF_SIZE * 2, 0, 128, &uart2_queue, 0);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_enable_pattern_det_baud_intr(UART, '+', PATTERN_CHR_NUM, 9, 0, 0);
    uart_pattern_queue_reset(UART, 20);

    xTaskCreatePinnedToCore(rx_task,
                            "rx_tsk",
                            UART_COMM_TASK_STACK_SIZE,
                            NULL,
                            UART_COMM_TASK_PRIORITY,
                            &uart_task_handle,
                            UART_COMM_TASK_CORE_ID);
}

void uart_deinit(void) {
    vTaskDelete(uart_task_handle);
    uart_flush_input(UART);
    uart_driver_delete(UART);
}

void _uart_deinit(void) {
    uart_flush_input(UART);
    uart_driver_delete(UART);
}

void uart_write(msg_t *msg) {
    const int txBytes = uart_write_bytes(UART, msg->data, msg->len);
    if (txBytes == msg->len) {
        ESP_LOGD(TAG, "write successful");
    }
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

bool uart_check_baud_rate(int baud_rate) {
    uart_config.baud_rate = baud_rate;

    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART, BUF_SIZE * 2, 0, 0, NULL, 0);

    return test_uart_comm();
}

int uart_detect_baud_rate(void) {
    int baud_rate = 115200;
    bool result = uart_check_baud_rate(baud_rate);
    if (!result) {
        _uart_deinit();
        baud_rate = 921600;
        result = uart_check_baud_rate(baud_rate);
        if (!result) {
            baud_rate = -1;
        }
    }
    _uart_deinit();
    return baud_rate;
}

bool test_uart_comm(void) {
    uint8_t data[] = {0xFF, 0x00, 0x0C, 0x1D, 0x03}; // Get Reader Runtime
    size_t data_len = 5;
    uart_write_bytes(UART, data, data_len);

    uint8_t buf[BUF_SIZE];
    int len = uart_read_bytes(UART, buf, BUF_SIZE, pdMS_TO_TICKS(50));
    if (len <= 0) {
        ESP_LOGE(TAG, "Failed to read data from UART");
        return false;
    }

    uint8_t expect_data[5] = {0xFF, 0x01, 0x0C, 0x00, 0x00};
    for (int i = 0; i < 5; ++i) {
        if (buf[i] != expect_data[i]) {
            return false;
        }
    }
    return true;
}
