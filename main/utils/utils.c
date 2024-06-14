//
// Created by Pratham Jaiswal on 20/03/24.
//

#include <stdlib.h>
#include <esp_err.h>
#include <esp_mac.h>
#include <esp_log.h>

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
    for (i = 0; i < msglen; ++i) {
        // NOTE: from silion docs it was for (i = 1; i < msglen; ++i) but as per my input command format I'm not taking headers from client
        CRC_calcCrc8(&calcCrc, MSG_CCITT_CRC_POLY, msgbuf[i]);
    }
    return calcCrc;
}

void rc_chk(int rc, char *msg) {
    if (rc == 0) {
        ESP_LOGD("rc_chk", "%s -> %d", msg, rc);
    } else {
        ESP_LOGE("rc_chk", "%s -> %d", msg, rc);
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
    *hex_str = (char *)malloc(*hex_str_len);
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
    *data = (uint8_t *)malloc(*len);
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