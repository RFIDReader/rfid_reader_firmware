//
// Created by Pratham Jaiswal on 10/04/24.
//

#include <esp_log.h>
#include <driver/i2c.h>

#include "battery.h"
#include "tasks_common.h"

static const char TAG[] = "bms";

battery_state_t battery_state;
TaskHandle_t bms_task_handle;

/**
 * @brief i2c command sequence
 *
 *        HOST: START - W ADDR       REG       REPEAT START - R ADDR              A          N - STOP
 *        IC:                  ↘ A ↗     ↘ A ↗                       ↘ A - DATA ↗   ↘ DATA ↗
 *
 *        W addr = (FUEL_GAUGE_ADDR << 1) | I2C_MASTER_WRITE
 *        R addr = (FUEL_GAUGE_ADDR << 1) | I2C_MASTER_READ
 *
 *        eg. for get soc
 *        START - 0xAA - ACK - 0x2C - ACK - 0xAB - ACK - 0x64 - NACK - STOP
 *                ^ W addr     ^ reg        ^ R addr     ^ data (soc)
 *
 * @param reg
 * @param single - whether response is single byte or two bytes
 * @return uint32_t data received from fuel gauge
 */
int fuel_gauge_read(uint8_t reg, bool single) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FUEL_GAUGE_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FUEL_GAUGE_ADDR << 1) | I2C_MASTER_READ, true);
    uint8_t b1 = 0, b2 = 0;
    i2c_master_read_byte(cmd, &b1, single);
    if (!single) {
        i2c_master_read_byte(cmd, &b2, false);
    }
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C read failed: %s", esp_err_to_name(ret));
        return -1;  // Indicate error
    }

    int data = b1;

    if (!single) {
        data = (((int) b2) << 8) | b1;
    }

    return data;
}

static void bms_r_task(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        battery_state.temp = bms_get_internal_temperature();
        vTaskDelay(pdMS_TO_TICKS(2500));
        battery_state.cycle_count = bms_get_cycle_count();
        vTaskDelay(pdMS_TO_TICKS(2500));
        battery_state.soc = bms_get_soc();
        vTaskDelay(pdMS_TO_TICKS(2500));
        battery_state.soh = bms_get_soh();
        vTaskDelay(pdMS_TO_TICKS(2500));

        ESP_LOGI(TAG, "TEMP: %d, SOC: %d, SOH: %d, Cycles: %d", battery_state.temp, battery_state.soc,
                 battery_state.soh, battery_state.cycle_count);
    }
}

void bms_init(void) {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = FUEL_GAUGE_SDA;
    conf.scl_io_num = FUEL_GAUGE_SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    i2c_param_config(I2C_PORT, &conf);

    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);

    xTaskCreatePinnedToCore(
            bms_r_task,
            "bms_r_tsk",
            BMS_TASK_STACK_SIZE,
            NULL,
            BMS_TASK_PRIORITY,
            &bms_task_handle,
            BMS_TASK_CORE_ID);
}

int bms_get_temperature(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_TEMP, false);
}

int bms_get_voltage(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_VOLT, true);
}

int bms_get_nom_available_cap(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_NOM_CAP, true);
}

int bms_get_full_available_cap(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_FULL_CAP, true);
}

int bms_get_remaining_available_cap(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_REM_CAP, true);
}

int bms_get_full_charge_cap(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_FULL_CHG_CAP, true);
}

int bms_get_avg_current(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_AVG_CUR, true);
}

int bms_get_time_to_empty(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_TTE, false);
}

int bms_get_safety_status(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_SAFETY_STAT, true);
}

int bms_get_imax(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_MAX_CUR, true);
}

int bms_get_internal_temperature(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_INT_TEMP, true);
}

int bms_get_cycle_count(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_CYC_CNT, true);
}

int bms_get_soc(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_SOC, true);
}

int bms_get_soh(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_SOH, true);
}

int bms_get_charging_v(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_CHG_VOLT, true);
}

int bms_get_charging_i(void) {
    return fuel_gauge_read(FUEL_GAUGE_REG_CHG_CUR, true);
}

void bms_deinit(void) {
    vTaskDelete(bms_task_handle);
    i2c_driver_delete(I2C_PORT);
}
