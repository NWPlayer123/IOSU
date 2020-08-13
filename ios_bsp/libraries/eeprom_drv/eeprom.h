#pragma once

enum EEPROM_WRITE_CONTROL {
    EEPROM_WRITE_DISABLE = 1,
    EEPROM_WRITE_ENABLE  = 2,
    EEPROM_WRITE_ERASE   = 3,
};

IOSError eepromDrvReadWord(int eeprom_ndx, uint8_t addr, uint16_t* data);
IOSError eepromDrvWriteWord(int eeprom_ndx, uint8_t addr, uint16_t data);
IOSError eepromDrvSetWriteControl(int eeprom_ndx, EEPROM_WRITE_CONTROL control);
