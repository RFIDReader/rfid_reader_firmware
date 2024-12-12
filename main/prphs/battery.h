//
// Created by Pratham Jaiswal on 10/04/24.
//

/// BQ27742-G1: https://www.ti.com/lit/ds/symlink/bq27742-g1.pdf
/// https://www.ti.com/lit/ug/sluuax0c/sluuax0c.pdf?ts=1718563280100

#ifndef RFID_READER_BATTERY_H
#define RFID_READER_BATTERY_H

#include "esp_event.h"
#include "driver/gpio.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#define FUEL_GAUGE_SDA GPIO_NUM_3
#define FUEL_GAUGE_SCL GPIO_NUM_2
#define FUEL_GAUGE_GPIO GPIO_NUM_18
#define FUEL_GAUGE_ADDR_LEN I2C_ADDR_BIT_LEN_7
#define FUEL_GAUGE_ADDR 0x55
#define FUEL_GAUGE_SPEED 400000
#define I2C_PORT I2C_NUM_1

#elif defined(CONFIG_IDF_TARGET_ESP32)

#define FUEL_GAUGE_SDA (-1)
#define FUEL_GAUGE_SCL (-1)
#define FUEL_GAUGE_GPIO (-1)

#endif

typedef struct {
    int soc;
    int soh;
    int cycle_count;
    int temp;
} battery_state_t;

#define FUEL_GAUGE_W_CMD 0xAA
#define FUEL_GAUGE_R_CMD 0xAB

typedef enum {
    FUEL_GAUGE_REG_TEMP = 0x06,               // Temperature Register
    FUEL_GAUGE_REG_VOLT = 0x08,               // Voltage Register
    FUEL_GAUGE_REG_NOM_CAP = 0x0C,            // Nominal Available Capacity Register
    FUEL_GAUGE_REG_FULL_CAP = 0x0E,           // Full Available Capacity Register
    FUEL_GAUGE_REG_REM_CAP = 0x10,            // Remaining Capacity Register
    FUEL_GAUGE_REG_FULL_CHG_CAP = 0x12,       // Full Charge Capacity Register
    FUEL_GAUGE_REG_AVG_CUR = 0x14,            // Average Current Register
    FUEL_GAUGE_REG_TTE = 0x16,                // Time to Empty Register
    FUEL_GAUGE_REG_SAFETY_STAT = 0x1A,        // Safety Status Register
    FUEL_GAUGE_REG_MAX_CUR = 0x1E,            // Maximum Current Register
    FUEL_GAUGE_REG_INT_TEMP = 0x28,           // Internal Temperature Register
    FUEL_GAUGE_REG_CYC_CNT = 0x2A,            // Cycle Count Register
    FUEL_GAUGE_REG_SOC = 0x2C,                // State of Charge Register
    FUEL_GAUGE_REG_SOH = 0x2E,                // State of Health Register
    FUEL_GAUGE_REG_CHG_VOLT = 0x30,           // Charge Voltage Register
    FUEL_GAUGE_REG_CHG_CUR = 0x32             // Charge Current Register
} fuel_gauge_regs_t;

extern battery_state_t battery_state;

void bms_init(void), bms_deinit(void);

int bms_get_temperature(void);

int bms_get_voltage(void);

int bms_get_nom_available_cap(void);

int bms_get_full_available_cap(void);

int bms_get_remaining_available_cap(void);

int bms_get_full_charge_cap(void);

int bms_get_avg_current(void);

int bms_get_time_to_empty(void);

int bms_get_safety_status(void);

int bms_get_imax(void);

int bms_get_internal_temperature(void);

int bms_get_cycle_count(void);

int bms_get_soc(void);

int bms_get_soh(void);

int bms_get_charging_v(void);

int bms_get_charging_i(void);

#endif //RFID_READER_BATTERY_H
