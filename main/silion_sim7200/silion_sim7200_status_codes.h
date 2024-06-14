//
// Created by Pratham Jaiswal on 10/04/24.
//

#ifndef RFID_READER_SILION_SIM7200_STATUS_CODES_H
#define RFID_READER_SILION_SIM7200_STATUS_CODES_H

#include <stdint.h>

typedef struct {
    uint16_t code;
    const char *message;
} status_code_t;

static const status_code_t STATUS_CODES[] = {
        {0x0000, "The operation is successful."},
        {0x0100, "The actual length of the data is different from the value of the length byte."},
        {0x0101, "Unavailable command"},
        {0x0105, "Unavailable parameter value"},
        {0x010A, "Unavailable baud rate"},
        {0x010B, "Unavailable region selection"},
        {0x0200, "App Firmware layer program CRC is incorrect"},
        {0x0302, "Flash not defined error, flash writing failed"},
        {0x0400, "No tag found"},
        {0x0402, "Protocol unavailable"},
        {0x040A, "General tag error (reading/writing lock, kill command)"},
        {0x040B, "Length of reading memory out of limit (e.g. only 96 words can be read at a time)"},
        {0x040C, "Unavailable kill password"},
        {0x0420, "GEN2 protocol error"},
        {0x0423, "MEMORY OVERRUN BAD PC"},
        {0x0424, "MEM LOCKED"},
        {0x042B, "INSUFFICIENT POWER"},
        {0x042F, "NON SPECIFIC ERROR"},
        {0x0430, "UNKNOWN ERROR"},
        {0x0500, "Unavailable frequency value"},
        {0x0504, "Temperature overrun"},
        {0x0505, "High return loss"},
        {0x7F00, "Unknown error, serious error"},
        {0xFF01, "Error occurs in initializing timer, reading/writing FLASH function, GPIO configuration function"},
        {0xFF02, "OEM initialization failed"},
        {0xFF03, "Reader command interface function initialization failed"},
        {0xFF04, "MAC register reading/writing function initialization failed"},
        {0xFF05, "MAC register initialization failed"},
        {0xFF06, "R2000 and ARM7 communication interface initialization failed"},
        {0xFF07, "R2000 and ARM7 communication detection failed"},
        {0xFF08, "R2000 and ARM7 communication detection failed"},
        {0xFF09, "GPIO configuration error"},
        {0xFF0A, "R2000 chip register initialization failed"},
        {0xFF0B, "EPC protocol function interface initialization failed"},
        {0xFF0C, "OEM mapping MAC register initialization failed"},
        {0xFF0D, "Serial port initialization failed"},
        {0xFF0E, "App main handler interface error"},
};

#define STATUS_CODES_COUNT (sizeof(STATUS_CODES) / sizeof(STATUS_CODES[0]))

#endif //RFID_READER_SILION_SIM7200_STATUS_CODES_H
