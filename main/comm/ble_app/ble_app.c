//
// Created by Pratham Jaiswal on 20/03/24.
//

#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "ble_app.h"
#include "settings.h"
#include "tasks_common.h"
#include "ble_prph.h"
#include "utils.h"
#include "comm_if.h"

static const char TAG[] = "ble_app";

esp_event_loop_handle_t ble_event_handle;

ESP_EVENT_DEFINE_BASE(BLE_APP_EVENTS);

static uint8_t own_addr_type;
static uint16_t conn_handle;
static uint8_t s_current_phy;

void ble_store_config_init(void);

static void ble_app_advertise(void);

static int gap_event(struct ble_gap_event *event, void *arg);

void host_task(void *param) {
    ESP_LOGI(TAG, "ble host task started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}

static void on_sync() {
    rc_chk(ble_svc_gap_device_name_set(settings.device_name), "ble_svc_gap_device_name_set");
    rc_chk(ble_hs_util_ensure_addr(0), "ble_hs_util_ensure_addr");
    rc_chk(ble_hs_id_infer_auto(0, &own_addr_type), "ble_hs_id_infer_auto");

#ifdef USE_2M_PHY
    s_current_phy = BLE_HCI_LE_PHY_1M_PREF_MASK;
    uint8_t prefered_phy = BLE_HCI_LE_PHY_1M_PREF_MASK; // | BLE_HCI_LE_PHY_1M_PREF_MASK | BLE_HCI_LE_PHY_CODED_PREF_MASK;
    rc_chk(ble_gap_set_prefered_default_le_phy(prefered_phy, prefered_phy), "ble_gap_set_prefered_default_le_phy");
#endif

    ble_app_advertise();
}

static void on_reset(int reason) {
    ESP_LOGI(TAG, "on_reset reason: %d", reason);
}

static void ble_app_advertise() {
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof fields);

    const char *name = ble_svc_gap_device_name();
    fields.name = (uint8_t *) name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    uint8_t mfg_data[] = {'p', 'r', '4', '7', 'h', '4', 'm'};
    fields.mfg_data = mfg_data;
    fields.mfg_data_len = 7;

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    fields.tx_pwr_lvl_is_present = 1;

//    fields.uuids16 = (ble_uuid16_t[]) {
//            BLE_UUID16_INIT(DEVICE_INFO_UUID16)
//    };
//    fields.num_uuids16 = 1;
//    fields.uuids16_is_complete = 1;

    rc_chk(ble_gap_adv_set_fields(&fields), "ble_gap_adv_set_fields");

    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof adv_params);

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc_chk(ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, gap_event, NULL), "ble_gap_adv_start");
}

static int gap_event(struct ble_gap_event *event, void *arg) {
    struct ble_gap_conn_desc desc;
    uint8_t tx_phy, rx_phy;
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            ESP_LOGI(TAG, "connection %s", event->connect.status == 0 ? "established" : "failed");
            if (event->connect.status == 0) {
                conn_handle = event->connect.conn_handle;
                rc_chk(ble_gap_conn_find(event->connect.conn_handle, &desc), "ble_gap_conn_find");
                esp_event_post_to(ble_event_handle,
                                  BLE_APP_EVENTS,
                                  BLE_EVENT_CONNECTED,
                                  NULL,
                                  0,
                                  portMAX_DELAY);
            } else {
                ESP_LOGE(TAG, "connection failed");
                ble_app_advertise();
            }
            return 0;

        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGE(TAG, "disconnect; reason=%d", event->disconnect.reason);
            ble_app_advertise();
            esp_event_post_to(ble_event_handle,
                              BLE_APP_EVENTS,
                              BLE_EVENT_DISCONNECTED,
                              NULL,
                              0,
                              portMAX_DELAY);
            /// If the rfid module has successfully started inventory and receives other valid commands, the module will exit continuous inventory and respond with an unsuccessful response containing error code 0xAA49.
            uint8_t tmp_cmd[] = {0xFF, 0x00, 0x03, 0x1D, 0x0C};
            msg_t tmp_msg = {
                    .data = tmp_cmd,
                    .len = 5,
            };
            comm_if_post(&tmp_msg);
            return 0;

        case BLE_GAP_EVENT_CONN_UPDATE:
            ESP_LOGI(TAG, "connection updated; status=%d", event->conn_update.status);
            rc_chk(ble_gap_conn_find(event->connect.conn_handle, &desc), "ble_gap_conn_find");
            return 0;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI(TAG, "advertisement complete; reason=%d", event->adv_complete.reason);
            vTaskDelay(pdMS_TO_TICKS(50));
            ble_app_advertise();
            return 0;

        case BLE_GAP_EVENT_ENC_CHANGE:
            /* Encryption has been enabled or disabled for this connection. */
            ESP_LOGI(TAG, "encryption change event; status=%d ", event->enc_change.status);
            rc_chk(ble_gap_conn_find(event->enc_change.conn_handle, &desc), "ble_gap_conn_find");
            return 0;

        case BLE_GAP_EVENT_NOTIFY_TX:
            ESP_LOGD(TAG, "notify_tx event; conn_handle=%d attr_handle=%d "
                          "status=%d is_indication=%d",
                     event->notify_tx.conn_handle,
                     event->notify_tx.attr_handle,
                     event->notify_tx.status,
                     event->notify_tx.indication);
            return 0;

        case BLE_GAP_EVENT_SUBSCRIBE:
            ESP_LOGD(TAG, "subscribe event; conn_handle=%d attr_handle=%d "
                          "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                     event->subscribe.conn_handle,
                     event->subscribe.attr_handle,
                     event->subscribe.reason,
                     event->subscribe.prev_notify,
                     event->subscribe.cur_notify,
                     event->subscribe.prev_indicate,
                     event->subscribe.cur_indicate);
            return 0;

        case BLE_GAP_EVENT_MTU:
            ESP_LOGI(TAG, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                     event->mtu.conn_handle,
                     event->mtu.channel_id,
                     event->mtu.value);
            return 0;

        case BLE_GAP_EVENT_REPEAT_PAIRING:
            rc_chk(ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc), "ble_gap_conn_find");
            ble_store_util_delete_peer(&desc.peer_id_addr);
            return BLE_GAP_REPEAT_PAIRING_RETRY;

        case BLE_GAP_EVENT_PASSKEY_ACTION:
            ESP_LOGI(TAG, "PASSKEY_ACTION_EVENT started");
            // https://github.com/espressif/esp-idf/blob/5a40bb8746633477c07ff9a3e90016c37fa0dc0c/examples/bluetooth/nimble/bleprph/main/main.c#L351
            return 0;

#ifdef USE_2M_PHY
        case BLE_GAP_EVENT_PHY_UPDATE_COMPLETE:
            rc_chk(ble_gap_read_le_phy(conn_handle, &tx_phy, &rx_phy), "ble_gap_read_le_phy");
            ESP_LOGI(TAG, "phy update complete tx_phy: %02X, rx_phy: %02X", tx_phy, rx_phy);
            if ((tx_phy & BLE_HCI_LE_PHY_2M_PREF_MASK) && (rx_phy & BLE_HCI_LE_PHY_2M_PREF_MASK)) {
                s_current_phy = BLE_HCI_LE_PHY_2M_PREF_MASK;
            }
            return 0;
#endif
        default:
            ESP_LOGW(TAG, "unhandled gap_event %d", event->type);
            return 0;
    }
    return 0;
}

void ble_app_init() {
    esp_event_loop_args_t loop_args = {
            .queue_size = BLE_APP_TASK_QUEUE_SIZE,
            .task_name = BLE_APP_TASK_NAME,
            .task_priority = BLE_APP_TASK_PRIORITY,
            .task_stack_size = BLE_APP_TASK_STACK_SIZE,
            .task_core_id = BLE_APP_TASK_CORE_ID,
    };

    esp_event_loop_create(&loop_args, &ble_event_handle);

    esp_err_t ret;
    ret = nimble_port_init();
    ESP_LOGI(TAG, "nimble_port_init -> %d", ret);

    ble_svc_gap_init();
    gatt_svr_init();

    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;
    ble_hs_cfg.sm_sc = 1;
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_our_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC;
    ble_hs_cfg.sm_their_key_dist |= BLE_SM_PAIR_KEY_DIST_ENC;

    ble_store_config_init();

    nimble_port_freertos_init(host_task);
}

void ble_app_send_msg(msg_t *msg) {
    gatt_svr_notify(conn_handle, msg);
}