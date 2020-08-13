static bool ccrhSummary; //.data:e6047980

static BSP_ATTRIBUTE bspCCRH_attributes[] = { //.data:e6044ccc
    {
        .pName = "Reset",
        .options = BSP_AO_INIT_GLOBAL,
        .permissions = BSP_PERMISSIONS_ALL,
        .data = 4,
        .dataSize = 4,
        .writeMethod = bspCCRH_Reset_write,
        .initMethod = bspCCRH_Reset_init,
    },
    {
        .pName = "Summary",
        .options = BSP_AO_QUERY_DATA_INDIRECT,
        .permissions = BSP_PERMISSIONS_ALL,
        .data = &ccrhSummary,
        .dataSize = sizeof(ccrhSummary),
    },
};

static BSP_ENTITY bspCCRH_entity = { //.rodata:e6040f78
    .pName = "CCRH",
    .version = 1,
    .instanceCount = 1,
    .options = BSP_EO_NONE,
    .pAttributes = &bspCCRH_attributes,
};

BSP_RVAL bspRegisterCCRH() { //.text:e6009d68
    BSP_RVAL ret;
    BSP_HARDWARE_VERSION hwver;

    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret != BSP_RVAL_OK) return;

    if (!BSP_IS_LATTE(hwver)) return BSP_RVAL_OK;

    if (BSP_IS_CAT(hwver) || BSP_IS_ID(hwver)) {
    /*  true for rev3 and beyond */
        ccrhSummary = !(bspBoardConfig.boardRevision < 2);
    } else if (BSP_IS_EV(hwver)) {
        ccrhSummary = !(bspBoardConfig.boardRevision < 4);
/*  This looks like a check for hardware newer than the ccrh module supports */
    } else if (BSP_GET_VARIANT(hwver) > 0x29 &&
               !BSP_IS_EV_Y(hwver)) {
        ccrhSummary = false;
    } else ccrhSummary = true;

    return bspRegisterEntity(&bspCCRH_entity);
}

BSP_RVAL bspCCRH_Reset_init(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pInitData, ulong size) {
    BSP_HARDWARE_VERSION hwver;
    BSP_RVAL ret;

    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret != BSP_RVAL_OK) return ret;

    //TODO hwver checks, similar to write

    int cmd = pAttribute->data;
    return bspCCRH_Reset_write(instance, pAttribute, cmd);
}

BSP_RVAL bspCCRH_Reset_write(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pWrittenData) {
    BSP_HARDWARE_VERSION hwver;
    BSP_RVAL ret;

    int cmd = *(int*)pWrittenData;
    int ccrio3 = 0, ccrio12 = 0;

    switch (cmd) {
    /*  Hold DRH in reset */
        case 1: break;
    /*  ? */
        case 2: {
            ccrio12 = 1;
            break;
        }
    /*  ? */
        case 3: {
            ccrio3 = 1;
            break;
        }
    /*  ? */
        case 4: break;
        default: {
            return BSP_RVAL_INVALID_PARAMETER;
        }
    }

    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret == BSP_RVAL_OK) {
        //trusting the decompiler on this one
        switch (BSP_GET_VARIANT(hwver)) {
            case BSP_VARIANT_EV_Y: {
                ret = bspWriteInternal("GPIO", 0, "CCRIO3", sizeof(ccrio3), ccrio3);
                break;
            }
            case BSP_VARIANT_CAT:
            case BSP_VARIANT_ID:
            case BSP_VARIANT_CAFE:
            case BSP_VARIANT_IH: {
                if (bspBoardConfig.boardRevision > 3) {
                    ret = bspWriteInternal("GPIO", 0, "CCRIO3", sizeof(ccrio3), ccrio3);
                }
                break;
            }
        }
    }
    BSP_SleepTimer(100); //.text:e600f5d0

    ret |= bspInitInternal("GPIO", 0, "CCRHReset", 0, NULL);

    uint32_t gpio_dir = *LT_GPIO_DIR;
    if (gpio_dir & 0x20 == 0) {
        ret |= bspInitInternal("GPIO", 0, "CCRIO12", 0, NULL);
    }

    int ccrhReset = 0;
    ret |= bspWriteInternal("GPIO", 0, "CCRHReset", sizeof(ccrhReset), &ccrhReset);
    if (cmd == 1) {
        log_debug_printf("holding DRH in reset\n");
        return ret;
    }

    int wifi_reset_pulse = 2;
    bspWriteInternal("SMC", 0, "DWIFIRSTPulse", sizeof(wifi_reset_pulse), &wifi_reset_pulse);
    bspWriteInternal("GPIO", 0, "CCRIO12", sizeof(ccrio12), &ccrio12);
    BSP_SleepTimer(0x1f5b58); //TODO wtf

    ccrhReset = 1;
    bspWriteInternal("GPIO", 0, "CCRHReset", sizeof(ccrhReset), &ccrhReset);

    //yup, they discard all that |= work, guess it only matters for cmd == 1
    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret != BSP_RVAL_OK) {
        log_debug_printf("resetting DRH\n");
        return ret;
    }

    if ( variant == BSP_VARIANT_EV_Y ||
        (variant == BSP_VARIANT_CAT &&
            bspBoardConfig.boardRevision >= 3 && bspBoardConfig.movPassiveReset == 1) ||
        (variant == BSP_VARIANT_ID &&
            bspBoardConfig.boardRevision >= 3 && bspBoardConfig.movPassiveReset == 1) ||
        (variant == BSP_VARIANT_CAFE &&
            bspBoardConfig.boardRevision >= 4) ||
        (variant == BSP_VARIANT_IH &&
            bspBoardConfig.boardRevision >= 4)) {
        //TODO what's the init_data mean
        uint16_t reset_init_data[5] = {0x2100, 0x0000, 0x0100, 0x0000, 0x0000};
        ret = bspInitInternal("GPIO", 0, "CCRHReset", sizeof(reset_init_data), reset_init_data);
        log_debug_printf("resetting DRH\n");
        return ret;
    }

    log_debug_printf("resetting DRH\n");
    return BSP_RVAL_OK;
}
