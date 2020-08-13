#include <eeprom_drv/eeprom.h>

//.text:e600c7ac
IOSError bcGet(BC_CONFIG* config) {
    IOSError err;

    if (!config) return IOS_ERROR_INVALID;

    uint16_t bcStructSize;
    err = eepromDrvReadWord(0, 0x1E, &bcStructSize);
    if (err != IOS_ERROR_OK) return err;

    bcStructSize /= 2; //size in 16-bit words
    if (bcStructSize > 0x24) {
        return IOS_ERROR_INVALID;
    }

    uint16_t data[0x24];
    memset(data, 0, sizeof(data));
    for (int i = 0; i < bcStructSize; i++) {
        err = eepromDrvReadWord(0, i + 0x1c, &data[i]);
        if (err != IOS_ERROR_OK) return err;
    }

    memcpy(config, data, 0x28); //note sizeof(data) = 0x48
    if (config->version == 0) {
        config->ddr3vendor = 0x5521; //"U!"
    }

    bcStructSize *= 2; //yes, really
    crc32Reset();
    uint32_t crc = crc32Input(&data[2], bcStructSize - 2);
    //endian could be wrong here
    if (crc != (data[0] << 16) | data[1]) {
        return IOS_ERROR_INVALID;
    }

    return IOS_ERROR_OK;
}