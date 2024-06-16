//
// Created by Pratham Jaiswal on 20/03/24.
//

#ifndef RFID_READER_SILION_SIM7200_H
#define RFID_READER_SILION_SIM7200_H

#include "esp_event.h"

#include "types.h"

#ifdef CONFIG_IDF_TARGET_ESP32S3

#define SHDN_PIN GPIO_NUM_6

#elif defined(CONFIG_IDF_TARGET_ESP32)

#define SHDN_PIN (-1)

#endif

void silion_sim7200_init(void);

void silion_sim7200_send(msg_t *msg);

void silion_sim7200_receive(msg_t *mst);

void silion_sim7200_boot_firmware(void);

void silion_sim7200_set_baud_rate(int baud_rate);

/// If the rfid module has successfully started inventory and receives other valid commands, the module will exit continuous inventory and respond with an unsuccessful response containing error code 0xAA49.
void silion_sim7200_stop_scanning(void);

#endif //RFID_READER_SILION_SIM7200_H
