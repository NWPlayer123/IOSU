#include <bsp_entity.h>
#include "eeprom_internal.h"

IOSError eepromDrvSetSK(eepromCtx* ctx, int state) {
    BSP_RVAL ret;

    if (state) state = 1;

    ret = bspWriteInternal("GPIO", 0, "EEPROM_SK", 4, &state);
    if (ret != BSP_RVAL_OK) {
        return IOS_ERROR_EXISTS;
    }

    return IOS_ERROR_OK;
}

IOSError eepromDrvSetCS(eepromCtx* ctx, int state) {
    BSP_RVAL ret;

    if (state) state = 1;

    ret = bspWriteInternal("GPIO", 0, "EEPROM_CS", 4, &state);
    if (ret != BSP_RVAL_OK) {
        return IOS_ERROR_EXISTS;
    }

    return IOS_ERROR_OK;
}

//.text:e600cbbc
IOSError eepromDrvSetDO(eepromCtx* ctx, int state) {
    BSP_RVAL ret;

    if (state) state = 1;

    ret = bspWriteInternal("GPIO", 0, "EEPROM_DO", 4, &state);
    if (ret != BSP_RVAL_OK) {
        return IOS_ERROR_EXISTS;
    }

    return IOS_ERROR_OK;
}

int eepromDrvReadBit(eepromCtx* ctx) {
    BSP_RVAL err;

/*  Clock EEPROM */
    eepromDrvSetSK(ctx, 0);
    eepromDrvSetCS(ctx, 1);
    udelay(4);
    eepromDrvSetSK(ctx, 1);
    udelay(4);

/*  Read out bit */
    int bit;
    err = bspReadInternal("GPIO", 0, "EEPROM_DI", 4, &bit);
    if (err != BSP_RVAL_OK) return 0;

    if (bit) bit = 1;
    return bit;
}

/*  Lowers CS and waits the given number of clocks */
//.text:e600cd1c
void eepromDrvFinishCommand(eepromCtx* ctx, int clocks) {
    eepromDrvSetSK(ctx, 0);
    eepromDrvSetCS(ctx, 0);
    eepromDrvSetDO(ctx, 0);

    if (clocks) {
        for (int i = 0; i < clocks; i++) {
            eepromDrvSetSK(ctx, 0);
            udelay(4);
            eepromDrvSetSK(ctx, 1);
            udelay(4);
        }
    }

    eepromDrvSetSK(ctx, 0);
}

/*  Waits at most 1000 clocks for DI to go high */
//.text:e600cf0c
IOSError eepromDrvWaitForReady(eepromCtx* ctx) {
    eepromDrvSetDO(ctx, 0);

    bool timeout = true;
    for (int i = 0; i < 1000; i++) {
        int bit = eepromDrvReadBit(ctx);
        if (bit != 0) {
            timeout = false;
            break;
        }
    }
    eepromDrvFinishCommand(ctx, 2);

    if (timeout) return IOS_ERROR_FAIL_INTERNAL;
    return IOS_ERROR_OK;
}

IOSError eepromDrvSendBits(eepromCtx* ctx, size_t size, uint32_t data) {
    if (size > 32) {
        return IOS_ERROR_INVALID;
    }

    //convert size to 0-based bit number
    size--;

    for (int i = size; i >= 0; i--) {
        eepromDrvSetSK(ctx, 0);
        eepromDrvSetCS(ctx, 1);
        eepromDrvSetDO(ctx, (data >> i) & 1);
        udelay(4);
        eepromDrvSetSK(ctx, 1);
        udelay(4);
    }

    return IOS_ERROR_OK;
}
