#pragma once

#include <ios_api/ios.h>

enum EEPROM_STATUS {
    EEPROM_STATUS_UNINITIALISED = 0,
    EEPROM_STATUS_INITIALISED   = 1,
    EEPROM_STATUS_OPEN          = 2,
    EEPROM_STATUS_CLOSED        = 3,
};

#define NUM_EEPROMS 1
typedef struct {
    int status;
} eepromCtx;

int eepromDrvReadBit(eepromCtx* ctx);
void eepromDrvFinishCommand(eepromCtx* ctx, int clocks);
IOSError eepromDrvWaitForReady(eepromCtx* ctx);
IOSError eepromDrvSendBits(eepromCtx* ctx, size_t size, uint32_t data);

IOSError eepromDrvSetDO(eepromCtx* ctx, int state);
IOSError eepromDrvSetCS(eepromCtx* ctx, int state);
IOSError eepromDrvSetSK(eepromCtx* ctx, int state);
