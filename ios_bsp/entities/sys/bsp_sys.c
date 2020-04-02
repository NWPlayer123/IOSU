
BSP_RVAL bspSys_ASICRevision(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pReadData) {
    BSP_RVAL ret;
    BSP_HARDWARE_VERSION hwver;
    BSP_SYSTEM_ASIC_REVISION asicRev = {
        .acrChipRevID = *HW_CHIPREVID;
    };

    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret != BSP_RVAL_OK) return ret;

    if (BSP_IS_LATTE(hwver)) {
        asicRev.ccrChipRevID = *LT_CHIPREVID;
    }

    memcpy(pReadData, asicRev, sizeof(asicRev));
}

BSP_RVAL bspSys_BoardRevision(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pReadData) {
    memcpy(pReadData, &bspBoardConfig.boardRevision, sizeof(bspBoardConfig.boardRevision));
    return BSP_RVAL_OK;
}

BSL_RVAL bspSys_ClockInfo(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pReadData) {
    return bspGetSystemClockInfo((BSP_SYSTEM_CLOCK_INFO*)pReadData);
}

BSP_RVAL bspSys_cpuUtilRead(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pReadData) {
    int cpuUtil;
    cpuUtil = IOS_IOPCPUUtilisation(); //UND 0x600; e600fc1c
    if (cpuUse < 0) return BSP_RVAL_OS_ERROR;

    memcpy(pReadData, &cpuUtil, sizeof(cpuUtil));
    return BSP_RVAL_OK;
}

BSP_RVAL bspSys_DevicePresence(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pReadData) {
    u32 val = 0x07f17317;
    if (bspBoardConfig.version > 4) {
        val = bspBoardConfig.devicePresence;
    }
    memcpy(pReadData, &val, sizeof(val));

    return BSP_RVAL_OK;
}

BSP_RVAL bspSys_iop2x_write(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pWrittenData) {
    BSP_RVAL ret;
    BSP_HARDWARE_VERSION hwver;
    bool iop2x = false;

    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret != BSP_RVAL_OK) return ret;

    if (!BSP_IS_LATTE(hwver)) return BSP_RVAL_OK;

    if (BSP_LATTE_REV(hwver) < BSP_LATTE_REV_A3X) return BSP_RVAL_OK;

    if (pWrittenData) {
        byte newIop = *(byte*)pWrittenData;

    /*  1x multiplier */
        if (newIop == 1) {
            iop2x = false;
    /*  2x multiplier */
        } else if (newIop == 2) {
            iop2x = true;
    /*  Yes, this is really ret =, and GetPowerMode uses |= later.
        I have no clue why. */
        } else ret = BSP_RVAL_INVALID_PARAMETER;
    } else {
        if (bspBoardConfig.iopClockMultiplier == 1) {
            iop2x = false;
        } else if (bspBoardConfig.iopClockMultiplier == 2) {
            iop2x = true;
        } else ret = BSP_RVAL_BOARD_CONFIG_INVALID;
    }

/*  NOTE: I have not figured out what the deal is with this yet.
    Checks whether the RAM (?) PLL is a specific configuration.
    bspSys_powerMode_read, .text:e6001a20 */
    int power;
    ret |= BSPGetPowerMode(&power);
    if (ret != BSP_RVAL_OK) return ret;

    if (power == 2) {
        iop2x = 0;
    }
    ret = IOS_SetIOP2xState(iop2x); //UND 0x6C0; e600fc7c
    if (ret != IOS_ERROR_OK) return BSP_RVAL_OS_ERROR;

    return BSP_RVAL_OK;
}

BSP_RVAL bspSys_iop2x_read(u32 instance, BSP_ATTRIBTUE* pAttribute, void* pReadData) {
    byte* iop2x = (byte*)pReadData;
    *iop2x = (*LT_IOP2X & LT_IOP2X_ENABLE) ? 2 : 1;

    return BSP_RVAL_OK;
}
