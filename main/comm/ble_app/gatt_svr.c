//
// Created by Pratham Jaiswal on 11/04/24.
//

#include <string.h>
#include <esp_log.h>

#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gatt/ble_svc_gatt.h"

#include "ble_prph.h"
#include "comm_if.h"
#include "utils.h"

static uint8_t *le_phy_val;
static uint16_t gatt_svr_chr_val_handle;
static uint16_t gatt_svr_manufacturer_name_chr_val_handle;
static uint16_t gatt_svr_tx_power_level_chr_val_handle;
static uint16_t gatt_svr_battery_level_chr_val_handle;
static uint16_t gatt_svr_read_chr_val_handle;
static uint16_t gatt_svr_write_chr_val_handle;

static const char TAG[] = "GATT_SVR";

static int
//chr_access_le_phy(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg),
svr_read(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg),
        svr_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg),
        chr_access_bat_lvl(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg),
        chr_access_man_name(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg),
        chr_access_tx_pwr_lvl(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
//        {
//                .type = BLE_GATT_SVC_TYPE_PRIMARY,
//                .uuid = BLE_UUID16_DECLARE(LE_PHY_UUID16),
//                .characteristics = (struct ble_gatt_chr_def[])
//                        {
//                                {
//                                        .uuid = BLE_UUID16_DECLARE(LE_PHY_CHR_UUID16),
//                                        .access_cb = chr_access_le_phy,
//                                        .val_handle = &gatt_svr_chr_val_handle,
//                                        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC | BLE_GATT_CHR_F_WRITE
//                                                 | BLE_GATT_CHR_F_WRITE_ENC,
//                                },
//                                {
//                                        0, /* No more characteristics in this service. */
//                                }
//                        },
//        },
        {
                .type = BLE_GATT_SVC_TYPE_PRIMARY,
                .uuid = BLE_UUID16_DECLARE(BATTERY_UUID16),
                .characteristics = (struct ble_gatt_chr_def[]) {
                        {
                                .uuid = BLE_UUID16_DECLARE(BATTERY_LEVEL_CHAR_UUID16),
                                .flags = BLE_GATT_CHR_F_READ,
                                .access_cb = chr_access_bat_lvl,
                                .val_handle = &gatt_svr_battery_level_chr_val_handle,
                        },
                        {0}, /* No more characteristics in this service. */
                },
        },
        {
                .type = BLE_GATT_SVC_TYPE_PRIMARY,
                .uuid = BLE_UUID16_DECLARE(DEVICE_INFO_UUID16),
                .characteristics = (struct ble_gatt_chr_def[]) {
                        {
                                .uuid = BLE_UUID16_DECLARE(MANUFACTURER_NAME_CHAR_UUID16),
                                .flags = BLE_GATT_CHR_F_READ,
                                .access_cb = chr_access_man_name,
                                .val_handle = &gatt_svr_manufacturer_name_chr_val_handle,
                        },
                        {0}, /* No more characteristics in this service. */
                },
        },
        {
                .type = BLE_GATT_SVC_TYPE_PRIMARY,
                .uuid = BLE_UUID16_DECLARE(TX_POWER_UUID16),
                .characteristics = (struct ble_gatt_chr_def[]) {
                        {
                                .uuid = BLE_UUID16_DECLARE(TX_POWER_LEVEL_CHAR_UUID16),
                                .flags = BLE_GATT_CHR_F_READ,
                                .access_cb = chr_access_tx_pwr_lvl,
                                .val_handle = &gatt_svr_tx_power_level_chr_val_handle,
                        },
                        {0}, /* No more characteristics in this service. */
                },
        },
        {
                .type = BLE_GATT_SVC_TYPE_PRIMARY,
                .uuid = SERVER_UUID128, // Define UUID for device type
                .characteristics = (struct ble_gatt_chr_def[]) {
                        {
                                .uuid = SERVER_READ_CHR_UUID128, // Define UUID for reading
                                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                                .access_cb = svr_read,
                                .val_handle = &gatt_svr_read_chr_val_handle,
                        },
                        {
                                .uuid = SERVER_WRITE_CHR_UUID128, // Define UUID for writing
                                .flags = BLE_GATT_CHR_F_WRITE,
                                .access_cb = svr_write,
                                .val_handle = &gatt_svr_write_chr_val_handle,
                        },
                        {0}, /* No more characteristics in this service. */
                },
        },
        {
                0, /* No more services. */
        },
};

//static int chr_access_le_phy(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
//    const ble_uuid_t *uuid;
//    int rand_num;
//    int rc;
//    int len;
//    uint16_t copied_len;
//    uuid = ctxt->chr->uuid;
//
//    /* Determine which characteristic is being accessed by examining its
//     * 128-bit UUID.
//     */
//
//    if (ble_uuid_cmp(uuid, BLE_UUID16_DECLARE(LE_PHY_CHR_UUID16)) == 0) {
//        switch (ctxt->op) {
//            case BLE_GATT_ACCESS_OP_READ_CHR:
//                rand_num = rand();
//                rc = os_mbuf_append(ctxt->om, &rand_num, sizeof rand_num);
//                return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
//
//            case BLE_GATT_ACCESS_OP_WRITE_CHR:
//                len = OS_MBUF_PKTLEN(ctxt->om);
//                if (len > 0) {
//                    le_phy_val = (uint8_t *) malloc(len * sizeof(uint8_t));
//                    if (le_phy_val) {
//                        rc = ble_hs_mbuf_to_flat(ctxt->om, le_phy_val, len, &copied_len);
//                        if (rc == 0) {
//                            MODLOG_DFLT(INFO, "Write received of len = %d", copied_len);
//                            return 0;
//                        } else {
//                            MODLOG_DFLT(ERROR, "Failed to receive write characteristic");
//                        }
//                    }
//                }
//                break;
//
//            default:
//                break;
//        }
//    }
//
//    /* Unknown characteristic; the nimble stack should not have called this
//     * function.
//     */
//    assert(0);
//    return BLE_ATT_ERR_UNLIKELY;
//}

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg) {
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
        case BLE_GATT_REGISTER_OP_SVC:
            ESP_LOGD(TAG, "registered service %s with handle=%d",
                     ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                     ctxt->svc.handle);
            break;

        case BLE_GATT_REGISTER_OP_CHR:
            ESP_LOGD(TAG, "registering characteristic %s with "
                          "def_handle=%d val_handle=%d",
                     ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                     ctxt->chr.def_handle,
                     ctxt->chr.val_handle);
            break;

        case BLE_GATT_REGISTER_OP_DSC:
            ESP_LOGD(TAG, "registering descriptor %s with handle=%d",
                     ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                     ctxt->dsc.handle);
            break;

        default:
            assert(0);
            break;
    }
}

int gatt_svr_init(void) {
    int rc;

    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0) {
        return rc;
    }

    return 0;
}

void gatt_svr_notify(uint16_t conn_handle, msg_t *msg) {
    struct os_mbuf *om = ble_hs_mbuf_from_flat(msg->data, msg->len);

    rc_chk(ble_gatts_notify_custom(conn_handle, gatt_svr_read_chr_val_handle, om), "ble_gatts_notify_custom");

    // os_mbuf_free(om); // if done this ESP crashes when second message is sent
}

static int svr_read(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (!ctxt || !ctxt->om) {
        ESP_LOGE(TAG, "Invalid access context or mbuf");
        return BLE_ATT_ERR_UNLIKELY;
    }
    os_mbuf_append(ctxt->om, "Use notifications", strlen("Use notifications"));
    return 0;
}

static int svr_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (!ctxt || !ctxt->om) {
        ESP_LOGE(TAG, "Invalid access context or mbuf");
        return BLE_ATT_ERR_UNLIKELY;
    }
    msg_t client_write = {ctxt->om->om_data, ctxt->om->om_len};
    comm_if_post(&client_write);
    return 0;
}

static int
chr_access_bat_lvl(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (!ctxt || !ctxt->om) {
        ESP_LOGE(TAG, "Invalid access context or mbuf");
        return BLE_ATT_ERR_UNLIKELY;
    }

    uint8_t value = 100;
    os_mbuf_append(ctxt->om, &value, sizeof(value));
    return 0;
}

static int
chr_access_man_name(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (!ctxt || !ctxt->om) {
        ESP_LOGE(TAG, "Invalid access context or mbuf");
        return BLE_ATT_ERR_UNLIKELY;
    }

    char *manufacturer_name = "pr47h4m";
    os_mbuf_append(ctxt->om, manufacturer_name, strlen(manufacturer_name));

    return 0;
}

static int
chr_access_tx_pwr_lvl(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (!ctxt || !ctxt->om) {
        ESP_LOGE(TAG, "Invalid access context or mbuf");
        return BLE_ATT_ERR_UNLIKELY;
    }

    uint8_t tx_power_level = 9;
    os_mbuf_append(ctxt->om, &tx_power_level, sizeof(tx_power_level));
    return 0;
}