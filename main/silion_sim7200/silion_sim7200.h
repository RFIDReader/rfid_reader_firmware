//
// Created by Pratham Jaiswal on 20/03/24.
//

#ifndef RFID_READER_SILION_SIM7200_H
#define RFID_READER_SILION_SIM7200_H

#include "esp_event.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#include "types.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#define UART UART_NUM_1
#define TXD_PIN GPIO_NUM_17
#define RXD_PIN GPIO_NUM_18
#define SHDN_PIN GPIO_NUM_6

#elif defined(CONFIG_IDF_TARGET_ESP32)

#define UART UART_NUM_2
#define TXD_PIN GPIO_NUM_17
#define RXD_PIN GPIO_NUM_16
#define SHDN_PIN (-1)

#endif

#define PATTERN_CHR_NUM (3)

static uart_config_t uart_config = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
};

void silion_sim7200_init(void);

void silion_sim7200_send(msg_t *msg);

void silion_sim7200_receive(msg_t *mst);

#endif //RFID_READER_SILION_SIM7200_H
