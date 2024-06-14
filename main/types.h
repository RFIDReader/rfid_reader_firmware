//
// Created by Pratham Jaiswal on 20/03/24.
//

#ifndef RFID_READER_TYPES_H
#define RFID_READER_TYPES_H

#include <stdint.h>

typedef struct {
    uint8_t *data;
    uint16_t len;
} msg_t;

#endif //RFID_READER_TYPES_H
