#include "bsp_api.h"

static BSP_RVAL bspDI_IOStrength_init(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pInitData, ulong size) {
    BSP_RVAL err;
    BSP_HARDWARE_VERSION hwver;
    uint32_t reg;

    err = bspMethodGetHardwareVersion(&hwver);
    if (err) return err;

/*  Read HW_IOSTRCTRL0 and clear DI bits */
    reg = *HW_IOSTRCTRL0;
    reg &= ~HW_IOSTRCTRL0_DI(HW_IOSTRCTRL0_MASK);

/*  Set DI bits as appropriate */
    if (BSP_IS_HOLLYWOOD(hwver) || BSP_IS_HOLLYWOOD_ES1(hwver)) {
        reg |= HW_IOSTRCTRL0_DI(1);
    } else if (BSP_IS_LATTE(hwver) || BSP_IS_BOLLYWOOD(hwver)) {
        reg |= HW_IOSTRCTRL0_DI(2);
    } else {
        return BSP_RVAL_UNKNOWN_HARDWARE_VERSION;
    }

    *HW_IOSTRCTRL0 = reg;
    return BSP_RVAL_OK;
}

static BSP_RVAL bspDI_IOPower_read(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pReadData) {
    uint32_t reg;

    reg = *HW_IOPWRCTRL;
    if (reg & HW_IOPWRCTRL_DI(1) && reg & HW_IOPWRCTRL_DI(2)) {
        *(uint8_t*)pReadData = 1;
    } else {
        *(uint8_t*)pReadData = 0;
    }

    return BSP_RVAL_OK;
}

static BSP_RVAL bspDI_IOPower_write(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pWrittenData) {
    uint32_t reg;

    reg = *HW_IOPWRCTRL;
    reg &= ~HW_IOPWRCTRL_DI(HW_IOPWRCTRL_MASK);

    if (*(uint8_t*)pWrittenData != 0) {
        reg |= HW_IOPWRCTRL_DI(1) | HW_IOPWRCTRL_DI(2);
    }

    *HW_IOPWRCTRL = reg;
    return BSP_RVAL_OK;
}

static BSP_RVAL bspDI_IOPower_init(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pInitData, ulong size) {
    uint8_t enable = (uint8_t)pAttribute->data;

/*  Sanitise: enable must be 0 or 1 */
    if (enable) enable = 1;

    bspDI_IOPower_write(instance, pAttribute, &enable);
}

static BSP_RVAL bspDI_Reset_read(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pReadData) {
    BSP_RVAL err;
    uint8_t state;

    err = bspMethodRead("Reset", 0, "RSTB_DIRSTB", sizeof(state), &state, BSP_CLIENT_INTERNAL);
/*  Inverted: pin state of 0 means device off, or "in reset".
    Reset is therefore enabled, so invert. */
    *(uint8_t*)pReadData = !state;

    return err;
}

static BSP_RVAL bspDI_Reset_write(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pWrittenData) {
    uint8_t state;

/*  Inverted: input "1" means reset asserted aka device off - Reset uses 0 for
    this state */
    state = !*(uint8_t*)pWrittenData;
    return bspMethodWrite("Reset", 0, "RSTB_DIRSTB", sizeof(state), &state, BSP_CLIENT_INTERNAL);
}

static BSP_ATTRIBUTE bspDI_attributes[] = { //.data:e60439d0
    {
        .pName = "IOStrength",
        .options = BSP_AO_INIT_GLOBAL,
        .permissions = BSP_PERMISSIONS_IOS,
        .initMethod = bspDI_IOStrength_init,
    },
    {
        .pName = "BaseAddress",
        .options = BSP_AO_QUERY_DATA_DIRECT,
        .permissions = BSP_PERMISSIONS_IOS,
        .data = 0x0d006000,
        .dataSize = 4,
    },
    {
        .pName = "IOPower",
        .options = BSP_AO_QUERY_DATA_DIRECT,
        .permissions = BSP_PERMISSIONS_IOS,
        .data = 1,
        .dataSize = 1,
        .readMethod = bspDI_IOPower_read,
        .writeMethod = bspDI_IOPower_write,
        .initMethod = bspDI_IOPower_init,
    },
    {
        .pName = "Reset",
        .permissions = BSP_PERMISSIONS_IOS,
        .dataSize = 1,
        .readMethod = bspDI_Reset_read,
        .writeMethod = bspDI_Reset_write,
    },
};

static BSP_ENTITY bspDI_entity = { //.rodata:e60409c4
    .name = "DI",
    .version = 1,
    .instanceCount = 1,
    .options = BSP_EO_NONE;
    .pAttributes = {
        &bspDI_attributes,
    },
};

BSP_RVAL bspDIInstall() {
    return bspRegisterEntity(&bspDI_entity);
}
