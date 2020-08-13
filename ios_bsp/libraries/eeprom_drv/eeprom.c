#include "eeprom_internal.h"
static eepromCtx eepromContexts[NUM_EEPROMS];

IOSError eepromDrvReadWord(int eeprom_ndx, uint8_t addr, uint16_t* data) {
    IOSError ret;

    if (!(eeprom_ndx < NUM_EEPROMS)) {
        return IOS_ERROR_MAX;
    }

    eepromCtx* ctx = eepromContexts[eeprom_ndx];
    if (ctx->status != EEPROM_STATUS_OPEN) {
        return IOS_ERROR_NOT_READY;
    }

    uint32_t cmd = 0x600 | addr; //READ
    ret = eepromDrvSendBits(ctx, 11, cmd);

    if (ret == IOS_ERROR_OK) {
        uint16_t read = 0;
        for (int offset = 15; offset >= 0; offset--) {
            int bit = eepromDrvReadBit(ctx);
            if (bit) bit = 1;

            read |= bit << offset;
        }

        *data = read;
    }

    eepromDrvFinishCommand(ctx, 2);
    return ret;
}

IOSError eepromDrvWriteWord(int eeprom_ndx, uint8_t addr, uint16_t data) {
    IOSError ret;

    if (!(eeprom_ndx < NUM_EEPROMS)) {
        return IOS_ERROR_MAX;
    }

    eepromCtx* ctx = eepromContexts[eeprom_ndx];
    if (ctx->status != EEPROM_STATUS_OPEN) {
        return IOS_ERROR_NOTREADY;
    }

    uint32_t cmd = 0x500 | addr; //WRITE
    ret = eepromDrvSendBits(ctx, 27, (cmd << 16) | data);
    eepromDrvFinishCommand(ctx, 2);
    if (ret != IOS_ERROR_OK) return ret;

    return eepromDrvWaitForReady(ctx);
}

IOSError eepromDrvSetWriteControl(int eeprom_ndx, EEPROM_WRITE_CONTROL control) {
    IOSError ret;

    if (!(eeprom_ndx < NUM_EEPROMS)) {
        return IOS_ERROR_MAX;
    }

    eepromCtx* ctx = eepromContexts[eeprom_ndx];
    if (ctx->status != EEPROM_STATUS_OPEN) {
        return IOS_ERROR_NOTREADY;
    }

    if (control == EEPROM_WRITE_ERASE) {
        uint32_t cmd = 0x480; //ERAL
        ret = eepromDrvSendBits(ctx, 11, cmd);
        eepromDrvFinishCommand(ctx, 2);
        if (ret != IOS_ERROR_OK) return ret;

        return eepromDrvWaitForReady(ctx);
    }

    uint32_t cmd;
    if (control == EEPROM_WRITE_DISABLE) {
        cmd = 0x400; //EWDS
    } else if (control == EEPROM_WRITE_ENABLE) {
        cmd = 0x4c0; //EWEN
    } else {
        return IOS_ERROR_INVALID;
    }

    ret = eepromDrvSendBits(ctx, 11, cmd);
    eepromDrvFinishCommand(ctx, 2);
    return ret;
}

IOSError eepromDrvInit(int eeprom_ndx) {
    IOSError err = IOS_ERROR_OK;

    eepromCtx* ctx = eepromContexts[eeprom_ndx];
    if (!(eeprom_ndx < NUM_EEPROMS)) {
        return IOS_ERROR_MAX;
    }

    memset(ctx, 0, sizeof(*ctx));
    err |= eepromDrvSetCS(ctx, 0);
    err |= eepromDrvSetSK(ctx, 0);
    err |= eepromDrvSetDO(ctx, 0);
    if (err != IOS_ERROR_OK) return err;

    ctx->status = EEPROM_STATUS_INITIALISED;
    return IOS_ERROR_OK;
}

//.text:e600ce0c
IOSError eepromDrvOpen(int eeprom_ndx) {
    eepromCtx* ctx = eepromContexts[eeprom_ndx];
    if (!(eeprom_ndx < NUM_EEPROMS)) {
        return IOS_ERROR_MAX;
    }

    if (ctx->status != EEPROM_STATUS_CLOSED &&
        ctx->status != EEPROM_STATUS_INITIALISED) {
        return IOS_ERROR_NOT_READY;
    }

    eepromDrvSetCS(ctx, 0);
    eepromDrvSetSK(ctx, 0);
    eepromDrvSetDO(ctx, 0);

    ctx->status = EEPROM_STATUS_OPEN;
    return IOS_ERROR_OK;
}

IOSError eepromDrvShutdown(int eeprom_ndx) {
    eepromCtx* ctx = eepromContexts[eeprom_ndx];
    if (!(eeprom_ndx < NUM_EEPROMS)) {
        return IOS_ERROR_MAX;
    }

    if (ctx->status != EEPROM_STATUS_OPEN) {
        return IOS_ERROR_NOT_READY;
    }

    eepromDrvSetCS(ctx, 0);
    eepromDrvSetSK(ctx, 0);
    eepromDrvSetDO(ctx, 0);

    ctx->status = EEPROM_STATUS_CLOSED;
    return IOS_ERROR_OK;
}
