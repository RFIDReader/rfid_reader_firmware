//
// Created by Pratham Jaiswal on 11/04/24.
//

#ifndef RFID_READER_BLE_PRPH_H
#define RFID_READER_BLE_PRPH_H

#include "types.h"

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;

#define BATTERY_UUID16                0x180F
#define BATTERY_LEVEL_CHAR_UUID16     0x2A19

#define DEVICE_INFO_UUID16            0x180A
#define MANUFACTURER_NAME_CHAR_UUID16 0x2A29
#define MODEL_NUM_CHAR_UUID16         0x2A24
#define SERIAL_NUM_CHAR_UUID16        0x2A25
#define HARDWARE_REV_CHAR_UUID16      0x2A27
#define FIRMWARE_REV_CHAR_UUID16      0x2A26
#define SOFTWARE_REV_CHAR_UUID16      0x2A28
#define SYSTEM_ID_CHAR_UUID16         0x2A23

#define TX_POWER_UUID16               0x1804
#define TX_POWER_LEVEL_CHAR_UUID16    0x2A07

// 70723437-6834-6D6A-3431-3577346C3031
#define SERVER_UUID128 BLE_UUID128_DECLARE(0x70, 0x72, 0x34, 0x37, 0x68, 0x34, 0x6d, 0x6a, 0x34, 0x31, 0x35, 0x77, 0x34, 0x6c, 0x30, 0x31)

// 70723437-6834-6D6A-3431-3577346C3032
#define SERVER_READ_CHR_UUID128 BLE_UUID128_DECLARE(0x70, 0x72, 0x34, 0x37, 0x68, 0x34, 0x6d, 0x6a, 0x34, 0x31, 0x35, 0x77, 0x34, 0x6c, 0x30, 0x32)

// 70723437-6834-6D6A-3431-3577346C3033
#define SERVER_WRITE_CHR_UUID128 BLE_UUID128_DECLARE(0x70, 0x72, 0x34, 0x37, 0x68, 0x34, 0x6d, 0x6a, 0x34, 0x31, 0x35, 0x77, 0x34, 0x6c, 0x30, 0x33)

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);

int gatt_svr_init(void);

void gatt_svr_notify(uint16_t conn_handle, msg_t *msg);

#endif //RFID_READER_BLE_PRPH_H
