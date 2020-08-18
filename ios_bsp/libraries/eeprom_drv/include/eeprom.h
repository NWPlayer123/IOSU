#pragma once

#include <ios_api/ios.h>
#include <stdint.h>

typedef enum EEPROM_WRITE_CONTROL {
    EEPROM_WRITE_DISABLE = 1,
    EEPROM_WRITE_ENABLE  = 2,
    EEPROM_WRITE_ERASE   = 3,
} EEPROM_WRITE_CONTROL;

IOSError eepromDrvInit(int eeprom_ndx);
IOSError eepromDrvOpen(int eeprom_ndx);
IOSError eepromDrvShutdown(int eeprom_ndx);

IOSError eepromDrvReadWord(int eeprom_ndx, uint8_t addr, uint16_t* data);
IOSError eepromDrvWriteWord(int eeprom_ndx, uint8_t addr, uint16_t data);
IOSError eepromDrvSetWriteControl(int eeprom_ndx, EEPROM_WRITE_CONTROL control);
