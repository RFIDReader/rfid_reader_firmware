//
// Created by Pratham Jaiswal on 20/03/24.
//
// Base MAC Address: https://github.com/espressif/esp-idf/blob/master/examples/system/base_mac_address/README.md
//

#ifndef RFID_READER_UTILS_H
#define RFID_READER_UTILS_H

#include <stdio.h>

#define MSG_CRC_INIT 0xFFFF
#define MSG_CCITT_CRC_POLY 0x1021

uint16_t CalcCRC(uint8_t *msgbuf, uint8_t msglen);

uint8_t GetSubCRC(uint8_t *msgbuf, int msglen);

/**
 * @param rc    return code of the function
 * @param msg   log message
 */
void rc_chk(int rc, char *msg);

// print and conversion utils
void print_hex_arr(uint8_t *data, size_t len),
        uint8_arr_to_hex_str(uint8_t *data, size_t len, char **hex_str, size_t *hex_str_len),
        hex_str_to_uint8_arr(char *hex_str, size_t hex_str_len, uint8_t **data, size_t *len);

void get_mac_addr(uint8_t wifi_sta_mac_addr[6], uint8_t wifi_soft_ap_mac_addr[6], uint8_t bt_mac_addr[6]);

#endif //RFID_READER_UTILS_H
