//
// Created by Pratham Jaiswal on 20/03/24.
//

#include <stdlib.h>
#include <esp_err.h>
#include <esp_mac.h>
#include <esp_log.h>
#include <string.h>

#include "utils.h"

void CRC_calcCrc8(uint16_t *crcReg, uint16_t poly, uint16_t u8Data) {
    uint16_t i;
    uint16_t xorFlag;
    uint16_t bit;
    uint16_t dcdBitMask = 0x80;
    for (i = 0; i < 8; i++) {
        xorFlag = *crcReg & 0x8000;
        *crcReg <<= 1;
        bit = ((u8Data & dcdBitMask) == dcdBitMask);
        *crcReg |= bit;
        if (xorFlag) {
            *crcReg = *crcReg ^ poly;
        }
        dcdBitMask >>= 1;
    }
}

uint16_t CalcCRC(uint8_t *msgbuf, uint8_t msglen) {
    uint16_t calcCrc = MSG_CRC_INIT;
    uint8_t i;
    for (i = 1; i < msglen; ++i) {
        CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY, msgbuf[i]);
    }
    return calcCrc;
}

uint8_t GetSubCRC(uint8_t *msgbuf, int msglen) {
    int temp = 0;
    for (int i = 0; i < msglen; i++)
        temp += msgbuf[i];
    return (unsigned char) (temp & 0x000000ff);
}

void rc_chk(int rc, char *msg, char **err_to_name) {
    if (rc == 0) {
        ESP_LOGD("rc_chk", "%s -> %d", msg, rc);
    } else {
        if (err_to_name != NULL && err_to_name[rc] != NULL) {
            ESP_LOGE("rc_chk", "%s -> %d (%s)", msg, rc, err_to_name[rc]);
        } else {
            ESP_LOGE("rc_chk", "%s -> %d", msg, rc);
        }
    }
}

void print_hex_arr(uint8_t *array, size_t length) {
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", array[i]);
    }
    printf("\n");
}

void uint8_arr_to_hex_str(uint8_t *data, size_t len, char **hex_str, size_t *hex_str_len) {
    *hex_str_len = len * 2 + 1;
    *hex_str = (char *) malloc(*hex_str_len);
    if (*hex_str == NULL) {
        return;
    }

    for (size_t i = 0; i < len; i++) {
        sprintf(*hex_str + i * 2, "%02X", data[i]);
    }

    (*hex_str)[len * 2] = '\0';
}

void hex_str_to_uint8_arr(char *hex_str, size_t hex_str_len, uint8_t **data, size_t *len) {
    if (hex_str_len % 2 != 0) {
        return;
    }

    *len = hex_str_len / 2;
    *data = (uint8_t *) malloc(*len);
    if (*data == NULL) {
        return;
    }

    for (size_t i = 0; i < *len; i++) {
        sscanf(hex_str + i * 2, "%02hhX", &(*data)[i]);
    }
}

void get_mac_addr(uint8_t wifi_sta_mac_addr[6], uint8_t wifi_soft_ap_mac_addr[6], uint8_t bt_mac_addr[6]) {
    uint8_t base_mac_addr[6] = {0};

    esp_err_t ret = esp_read_mac(base_mac_addr, ESP_MAC_EFUSE_FACTORY);

    if (ret != ESP_OK) {
        ESP_LOGE("Utils", "Failed to get base MAC address from EFUSE BLK0. (%s)", esp_err_to_name(ret));
    }

    esp_iface_mac_addr_set(base_mac_addr, ESP_MAC_BASE);

    ESP_ERROR_CHECK(esp_read_mac(wifi_sta_mac_addr, ESP_MAC_WIFI_STA));

    ESP_ERROR_CHECK(esp_read_mac(wifi_soft_ap_mac_addr, ESP_MAC_WIFI_SOFTAP));

    ESP_ERROR_CHECK(esp_read_mac(bt_mac_addr, ESP_MAC_BT));
}

//msg_t *merge_msg(msg_t *msg1, msg_t *msg2) {
//    size_t len = msg1->len + msg2->len;
//    uint8_t *data = (uint8_t *) malloc(sizeof(uint8_t) * len);
//    memcpy(data, msg1->data, msg1->len);
//    memcpy(data + msg1->len, msg2->data, msg2->len);
//
//    msg_t *msg = (msg_t *) malloc(sizeof(msg_t));
//    msg->data = data;
//    msg->len = len;
//
//    return msg;
//}
//
//void free_msg(msg_t *msg) {
//    if (msg == NULL) {
//        return;
//    }
//    free(msg->data);
//    free(msg);
//}

msg_t *merge_msgs(msg_t *msgs[], int num_msgs) {
    // Calculate total length of the merged data
    size_t total_len = 0;
    for (int i = 0; i < num_msgs; i++) {
        total_len += msgs[i]->len;
    }

    // Allocate memory for the merged data
    uint8_t *data = (uint8_t *) malloc(total_len);
    if (data == NULL) {
        return NULL;  // Handle memory allocation failure
    }

    // Copy each message's data into the merged buffer
    size_t offset = 0;
    for (int i = 0; i < num_msgs; i++) {
        memcpy(data + offset, msgs[i]->data, msgs[i]->len);
        offset += msgs[i]->len;
    }

    // Allocate memory for the merged message structure
    msg_t *merged_msg = (msg_t *) malloc(sizeof(msg_t));
    if (merged_msg == NULL) {
        free(data);  // Clean up allocated data on failure
        return NULL;
    }
    merged_msg->data = data;
    merged_msg->len = total_len;

    return merged_msg;
}

void free_msg(msg_t *msg) {
    if (msg == NULL) {
        return;
    }
    free(msg->data);
    free(msg);
}
