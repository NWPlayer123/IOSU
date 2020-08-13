#include <eeprom_drv/eeprom.h>
#include <bc/bc.h>

static BSP_ATTRIBUTE bspEE_attributes[] = { //.data:e6044da8
    {
        .pName = "access",
        .permissions = BSP_PERMISSIONS_IOS_SUPV,
        .dataSize = 2,
        .readMethod = bspEE_access_read,
        .writeMethod = bspEE_access_write,
    },
    {
        .pName = "control",
        .options = BSP_AO_INIT_GLOBAL,
        .permissions = BSP_PERMISSIONS_IOS_SUPV,
        .dataSize = 4,
        .writeMethod = bspEE_control_write,
        .initMethod = bspEE_control_init,
        .shutdownMethod = bspEE_control_shutdown,
    },
    {
        .pName = "bc",
        .permissions = BSP_PERMISSIONS_IOS_SUPV,
        .dataSize = 0x28,
        .readMethod = bspEE_bc_read,
    },
};

static BSP_ENTITY bspEE_entity = { //.rodata:e6040fcc
    .name = "EE",
    .version = 1,
    .instanceCount = 0x100,
    .options = BSP_EO_FLAT_INSTANCES;
    .pAttributes = {
        &bspEE_attributes,
    },
};

//.text:e600a480
BSP_RVAL bspEE_access_read(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pReadData) {
    IOSError err;
    uint8_t addr = instance & 0xff;
    uint16_t data;

    err = eepromDrvReadWord(0, addr, &data);
    if (err != IOS_ERROR_OK) {
        return BSP_RVAL_DEVICE_ERROR;
    }

    *(uint16_t*)pReadData = data;
    return BSP_RVAL_OK;
}

//.text:e600a45c
BSP_RVAL bspEE_access_write(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pWrittenData) {
    IOSError err;
    uint8_t addr = instance & 0xff;
    uint16_t data = *(uint16_t*)pWrittenData;

    err = eepromDrvWriteWord(0, addr, data);
    if (err != IOS_ERROR_OK) {
        return BSP_RVAL_DEVICE_ERROR;
    }

    return BSP_RVAL_OK;
}

//.text:e600a3dc
BSP_RVAL bspEE_control_write(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pWrittenData) {
    int cmd_in = *(int*)pWrittenData;
    int cmd;
    IOSError err;

    if (cmd_in == 1) {
        cmd = EEPROM_WRITE_ENABLE;
    } else if (cmd_in == 2) {
        cmd = EEPROM_WRITE_DISABLE;
    } else {
        return BSP_RVAL_INVALID_PARAMETER;
    }

    err = eepromDrvSetWriteControl(0, cmd);
    if (err != IOS_ERROR_OK) return BSP_RVAL_DEVICE_ERROR;

    return BSP_RVAL_OK;
}

//.text:e600a424
BSP_RVAL bspEE_control_init(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pInitData, ulong size) {
    IOSError err;

    err = eepromDrvInit(0);
    if (err != IOS_ERROR_OK) return BSP_RVAL_DEVICE_ERROR;

    err = eepromDrvOpen(0);
    if (err != IOS_ERROR_OK) return BSP_RVAL_DEVICE_ERROR;

    err = eepromDrvSetWriteControl(0, EEPROM_WRITE_DISABLE);
    if (err != IOS_ERROR_OK) return BSP_RVAL_DEVICE_ERROR;

    return BSP_RVAL_OK;
}

//.text:e600a3c0
BSP_RVAL bspEE_control_shutdown(u32 instance, BSP_ATTRIBUTE* pAttribute) {
    IOSError err;

    err = eepromDrvShutdown(0);
    if (err != IOS_ERROR_OK) {
        return BSP_RVAL_DEVICE_ERROR;
    }

    return BSP_RVAL_OK;
}

BSP_RVAL bspEE_bc_read(u32 instance, BSP_ATTRIBUTE* attribute, void* pReadData) {
    BC_CONFIG* bc = (BC_CONFIG*)pReadData;
    IOSError err;

    err = bcGet(bc);
    if (err != IOS_ERROR_OK) {
        return BSP_RVAL_CFG_CORRUPTED;
    }

    return BSP_RVAL_OK;
}
