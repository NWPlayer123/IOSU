BSP_RVAL bspGPIO_IOStrength_init(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pInitData, ulong size) {
    BSP_HARDWARE_VERSION hwver;
    BSP_RVAL ret;

    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret != BSP_RVAL_OK) return ret;

    uint32_t reg = *HW_IOSTRCTRL1;
}

/*  Installs GPIO entity
 *  .text:e6006800
 */
int bspGPIOInstall() {
    int ret;
    BSP_HARDWARE_VERSION hwver;

    ret = bspMethodGetHardwareVersion(&hwver); //.text:E600B62C - see below
    if (ret != BSP_RVAL_OK) return ret;

    if (!BSP_IS_LATTE(hwver)) {
        return BSP_RVAL_UNKNOWN_HARDWARE_VERSION;
    }

    return bspRegisterEntity(&gpio_latte_entity);
}
