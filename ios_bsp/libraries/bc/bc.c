#include <bc.h>

#include <string.h>

#include <eeprom.h>
#include <crc32.h>
#include <log.h>

static int bc_log_handle = -1; //.data:e604696c
static bool bc_initialised; //.bss:e60479d0

//.text:e600c88c
IOSError bcInit() {
    IOSError err;

    bc_log_handle = log_open("BC", 0, 1);

    if (bc_initialised == false) {
        err = eepromDrvInit(0);
        if (err == IOS_ERROR_OK) {
            err = eepromDrvOpen(0);
            if (err == IOS_ERROR_OK) {
                bc_initialised = true;
            }
        }

        crc32MakeTable();
        return err;
    }

    return IOS_ERROR_OK;
}

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
        config->ddr3Vendor = 0x5521; //"U!"
    }

    bcStructSize *= 2; //yes, really
    crc32Reset();
    uint32_t crc = crc32Input(&data[2], bcStructSize - 2);
    //endian could be wrong here
    if (crc != ((data[0] << 16) | data[1])) {
        return IOS_ERROR_INVALID;
    }

    return IOS_ERROR_OK;
}
