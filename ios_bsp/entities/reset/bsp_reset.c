
BSP_RVAL bspReset_LT_RSTB_read(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pReadData) {
    uint32_t reg = *LT_RSTB;
    uint32_t bit = reg & pAttribute->attribute_specific;

    if (bit != 0) {
        *(uint8_t*)pReadData = 1;
    } else {
        *(uint8_t*)pReadData = 0;
    }

    return BSP_RVAL_OK;
}

BSP_RVAL bspReset_LT_RSTB_write(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pWrittenData) {
    uint8_t cmd = *(uint8_t*)pWrittenData;
    uint32_t reg = *LT_RSTB;

    if (cmd == 0) {
        reg &= ~pAttribute->attribute_specific;
    } else {
        reg |=  pAttribute->attribute_specific;
    }

    *LT_RSTB = reg;
    return BSP_RVAL_OK;
}
