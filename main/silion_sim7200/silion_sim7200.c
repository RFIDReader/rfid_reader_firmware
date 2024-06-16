//
// Created by Pratham Jaiswal on 20/03/24.
//

#include <esp_log.h>

#include "silion_sim7200.h"
#include "silion_sim7200_status_codes.h"
#include "comm_if.h"
#include "tasks_common.h"
#include "uart.h"
#include "utils.h"

static const char TAG[] = "silion_sim7200";

ESP_EVENT_DEFINE_BASE(SILION_SIM7200_EVENTS);
esp_event_loop_handle_t silion_sim7200_event_handle;

void silion_sim7200_send(msg_t *msg) {
    ESP_LOGD(TAG, "silion_sim7200_post");
    // format to silion command
    /// header (1byte) (FF) - data size (1byte) - command code (1byte) - data (n bytes) - crc16 (2bytes)
    uart_write(msg);
}

void silion_sim7200_receive(msg_t *msg) {
    // format of silion response
    /// header (1byte) (FF) - data size (1byte) - command code (1byte) - status code (2bytes) - data (n bytes) - crc16 (2bytes)
    // so if message is received: header, data size, command code, status code, crc16 is minimum expected which is 7 bytes
    comm_if_receive(msg);
}

void silion_sim7200_init() {
    uart_init();

    silion_sim7200_boot_firmware();

    esp_event_loop_args_t loop_args = {
            .queue_size = RFID_MODULE_TASK_QUEUE_SIZE,
            .task_name = RFID_MODULE_TASK_NAME,
            .task_priority = RFID_MODULE_TASK_PRIORITY,
            .task_stack_size = RFID_MODULE_TASK_STACK_SIZE,
            .task_core_id = RFID_MODULE_TASK_CORE_ID,
    };

    esp_event_loop_create(&loop_args, &silion_sim7200_event_handle);
}

void silion_sim7200_boot_firmware(void) {
    // boot firmware
    uint8_t boot_firmware_data[] = {0xFF, 0x00, 0x04, 0x1D, 0x0B};
    msg_t boot_firmware = {
            .data = boot_firmware_data,
            .len = 5,
    };
    uart_write(&boot_firmware);
}

void silion_sim7200_set_baud_rate(int baud_rate) {
    uint8_t baud_rate_hex[4] = {0};
    if (baud_rate == 115200) {
        baud_rate_hex[0] = 0x00;
        baud_rate_hex[1] = 0x01;
        baud_rate_hex[2] = 0xC2;
        baud_rate_hex[3] = 0x00;
    } else if (baud_rate == 921600) {
        baud_rate_hex[0] = 0x00;
        baud_rate_hex[1] = 0x0E;
        baud_rate_hex[2] = 0x10;
        baud_rate_hex[3] = 0x00;
    }

    uint8_t sub_command[8] = {0xAA, 0x40, 0x06, 0x01, baud_rate_hex[0], baud_rate_hex[1], baud_rate_hex[2],
                             baud_rate_hex[3]};
    uint8_t subCrc = GetSubCRC(sub_command, 8);
    uint8_t p = 0;
    uint8_t cmd[255] = {0};
    uint8_t moduletech[10] = {0x4D, 0x6F, 0x64, 0x75, 0x6C, 0x65, 0x74, 0x65, 0x63, 0x68};
    cmd[p++] = 0xFF;
    cmd[p++] = 0x14; // data_len
    cmd[p++] = 0xAA;
    for (int i = 0; i < 10; ++i) {
        cmd[p++] = moduletech[i];
    }
    for (int i = 0; i < 8; ++i) {
        cmd[p++] = sub_command[i];
    }
    cmd[p++] = subCrc;
    cmd[p++] = 0xBB; // terminator
    uint16_t crc16 = CalcCRC(cmd, p);
    cmd[p++] = (uint8_t) (crc16 >> 8);
    cmd[p++] = (uint8_t) (crc16 & 0xFF);

    printf("set baud rate: ");
    print_hex_arr(cmd, p);

    msg_t msg = {
            .data = cmd,
            .len = p,
    };
    uart_write(&msg);
}

void silion_sim7200_stop_scanning(void) {
    uint8_t tmp_cmd[] = {0xFF, 0x00, 0x03, 0x1D, 0x0C};
    msg_t tmp_msg = {
            .data = tmp_cmd,
            .len = 5,
    };
    uart_write(&tmp_msg);
}
