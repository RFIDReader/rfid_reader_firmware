//
// Created by Pratham Jaiswal on 10/04/24.
//

#ifndef RFID_READER_SILION_SIM7200_COMMANDS_H
#define RFID_READER_SILION_SIM7200_COMMANDS_H

#include <stdint.h>

static const uint8_t BOOT_FIRMWARE = 0x04;
static const uint8_t GET_VERSION = 0x03;
static const uint8_t BOOT_BOOTLOADER = 0x09;
static const uint8_t GET_SERIAL_NUMBER = 0x10;
static const uint8_t SINGLE_TAG_INVENTORY = 0x21;
static const uint8_t SYNCHRONOUS_INVENTORY = 0x22;
static const uint8_t GET_TAG_BUFFER = 0x29;
static const uint8_t ASYNCHRONOUS_INVENTORY = 0xAA;
static const uint8_t WRITE_TAG_DATA = 0x24;
static const uint8_t WRITE_TAG_EPC = 0x23;
static const uint8_t LOCK_TAG = 0x25;
static const uint8_t KILL_TAG = 0x26;
static const uint8_t READ_TAG_DATA = 0x28;
static const uint8_t SET_ANTENNA_PORTS = 0x91;
static const uint8_t SET_CURRENT_TAG_PROTOCOL = 0x93;
static const uint8_t SET_FREQUENCY_HOPPING = 0x95;
static const uint8_t SET_GPO = 0x96;
static const uint8_t SET_CURRENT_REGION = 0x97;
static const uint8_t SET_READER_CONFIGURATION = 0x9A;
static const uint8_t SET_PROTOCOL_CONFIGURATION = 0x9B;
static const uint8_t GET_ANTENNA_PORTS = 0x61;
static const uint8_t GET_TAG_PROTOCOL = 0x63;
static const uint8_t GET_FREQUENCY_HOPPING = 0x65;
static const uint8_t GET_GPI = 0x66;
static const uint8_t GET_CURRENT_REGION = 0x67;
static const uint8_t GET_AVAILABLE_REGIONS = 0x71;
static const uint8_t GET_READER_CONFIGURATION = 0x6A;
static const uint8_t GET_PROTOCOL_CONFIGURATION = 0x6B;
static const uint8_t GET_CURRENT_TEMPERATURE = 0x72;

static const uint8_t SILION_COMMANDS[] = {
        GET_VERSION,
        BOOT_FIRMWARE,
        BOOT_BOOTLOADER,
        GET_SERIAL_NUMBER,
        SINGLE_TAG_INVENTORY,
        SYNCHRONOUS_INVENTORY,
        GET_TAG_BUFFER,
        ASYNCHRONOUS_INVENTORY,
        WRITE_TAG_DATA,
        WRITE_TAG_EPC,
        LOCK_TAG,
        KILL_TAG,
        READ_TAG_DATA,
        SET_ANTENNA_PORTS,
        SET_CURRENT_TAG_PROTOCOL,
        SET_FREQUENCY_HOPPING,
        SET_GPO,
        SET_CURRENT_REGION,
        SET_READER_CONFIGURATION,
        SET_PROTOCOL_CONFIGURATION,
        GET_ANTENNA_PORTS,
        GET_TAG_PROTOCOL,
        GET_FREQUENCY_HOPPING,
        GET_GPI,
        GET_CURRENT_REGION,
        GET_AVAILABLE_REGIONS,
        GET_READER_CONFIGURATION,
        GET_PROTOCOL_CONFIGURATION,
        GET_CURRENT_TEMPERATURE,
};

#define SILION_COMMANDS_COUNT (sizeof(SILION_COMMANDS) / sizeof(SILION_COMMANDS[0]))

#endif //RFID_READER_SILION_SIM7200_COMMANDS_H
