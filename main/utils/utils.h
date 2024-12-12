//
// Created by Pratham Jaiswal on 20/03/24.
//
// Base MAC Address: https://github.com/espressif/esp-idf/blob/master/examples/system/base_mac_address/README.md
//

#ifndef RFID_READER_UTILS_H
#define RFID_READER_UTILS_H

#include <stdio.h>
#include <host/ble_hs.h>
#include "types.h"

#define MSG_CRC_INIT 0xFFFF
#define MSG_CCITT_CRC_POLY 0x1021

uint16_t CalcCRC(uint8_t *msgbuf, uint8_t msglen);

uint8_t GetSubCRC(uint8_t *msgbuf, int msglen);

static const char *ble_err_to_name[] = {
        [BLE_HS_EAGAIN] = "BLE_HS_EAGAIN",
        [BLE_HS_EALREADY] = "BLE_HS_EALREADY",
        [BLE_HS_EINVAL] = "BLE_HS_EINVAL",
        [BLE_HS_EMSGSIZE] = "BLE_HS_EMSGSIZE",
        [BLE_HS_ENOENT] = "BLE_HS_ENOENT",
        [BLE_HS_ENOMEM] = "BLE_HS_ENOMEM",
        [BLE_HS_ENOTCONN] = "BLE_HS_ENOTCONN",
        [BLE_HS_ENOTSUP] = "BLE_HS_ENOTSUP",
        [BLE_HS_EAPP] = "BLE_HS_EAPP",
        [BLE_HS_EBADDATA] = "BLE_HS_EBADDATA",
        [BLE_HS_EOS] = "BLE_HS_EOS",
        [BLE_HS_ECONTROLLER] = "BLE_HS_ECONTROLLER",
        [BLE_HS_ETIMEOUT] = "BLE_HS_ETIMEOUT",
        [BLE_HS_EDONE] = "BLE_HS_EDONE",
        [BLE_HS_EBUSY] = "BLE_HS_EBUSY",
        [BLE_HS_EREJECT] = "BLE_HS_EREJECT",
        [BLE_HS_EUNKNOWN] = "BLE_HS_EUNKNOWN",
        [BLE_HS_EROLE] = "BLE_HS_EROLE",
        [BLE_HS_ETIMEOUT_HCI] = "BLE_HS_ETIMEOUT_HCI",
        [BLE_HS_ENOMEM_EVT] = "BLE_HS_ENOMEM_EVT",
        [BLE_HS_ENOADDR] = "BLE_HS_ENOADDR",
        [BLE_HS_ENOTSYNCED] = "BLE_HS_ENOTSYNCED",
        [BLE_HS_EAUTHEN] = "BLE_HS_EAUTHEN",
        [BLE_HS_EAUTHOR] = "BLE_HS_EAUTHOR",
        [BLE_HS_EENCRYPT] = "BLE_HS_EENCRYPT",
        [BLE_HS_EENCRYPT_KEY_SZ] = "BLE_HS_EENCRYPT_KEY_SZ",
        [BLE_HS_ESTORE_CAP] = "BLE_HS_ESTORE_CAP",
        [BLE_HS_ESTORE_FAIL] = "BLE_HS_ESTORE_FAIL",
        [BLE_HS_EPREEMPTED] = "BLE_HS_EPREEMPTED",
        [BLE_HS_EDISABLED] = "BLE_HS_EDISABLED",
        [BLE_HS_ESTALLED] = "BLE_HS_ESTALLED",
};

/**
 * @param rc    return code of the function
 * @param msg   log message
 */
void rc_chk(int rc, char *msg, char** err_to_name);

// print and conversion utils
void print_hex_arr(uint8_t *data, size_t len),
        uint8_arr_to_hex_str(uint8_t *data, size_t len, char **hex_str, size_t *hex_str_len),
        hex_str_to_uint8_arr(char *hex_str, size_t hex_str_len, uint8_t **data, size_t *len);

void get_mac_addr(uint8_t wifi_sta_mac_addr[6], uint8_t wifi_soft_ap_mac_addr[6], uint8_t bt_mac_addr[6]);

msg_t *merge_msgs(msg_t *msgs[], int num_msgs);

void free_msg(msg_t *msg);

#endif //RFID_READER_UTILS_H
