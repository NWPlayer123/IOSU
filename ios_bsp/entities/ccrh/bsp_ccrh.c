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

    if (BSP_IS_CAT(hwver) | BSP_IS_ID(hwver)) {
    /*  true for rev3 and beyond */
        ccrhSummary = !(bspBoardConfig.boardRevision < 2);
    } else if (BSP_IS_EV(hwver)) {
        ccrhSummary = !(bspBoardConfig.boardRevision < 4);
/*  This looks like a check for hardware newer than the ccrh module supports */
    } else if (/*board variant 0x2A or above*/ &&
               !BSP_IS_EV_Y(hwver)) {
        ccrhSummary = false;
    } else ccrhSummary = true;

    return bspRegisterEntity(&bspCCRH_entity);
}
