#pragma once

#define NUM_EEPROMS 1
typedef struct {
    int status;
} eepromCtx;

int eepromDrvReadBit(eepromCtx* ctx);
void eepromDrvFinishCommand(eepromCtx* ctx, int clocks);
IOSError eepromDrvWaitForReady(eepromCtx* ctx);

IOSError eepromDrvSetDO(eepromCtx* ctx, int state);
IOSError eepromDrvSetCS(eepromCtx* ctx, int state);
IOSError eepromDrvSetSK(eepromCtx* ctx, int state);
