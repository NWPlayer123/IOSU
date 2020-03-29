#include "bsp_api.h"

static BSP_RVAL bspSI_IOStrength_init(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pInitData, ulong size) {
    BSP_RVAL err;
    BSP_HARDWARE_VERSION hwver;
    uint32_t reg;

    err = bspMethodGetHardwareVersion(&hwver);
    if (err) return err;

/*  Read HW_IOSTRCTRL0 and clear SI bits */
    reg = *HW_IOSTRCTRL0;
    reg &= ~HW_IOSTRCTRL0_SI(HW_IOSTRCTRL0_MASK);

/*  Set SI bits as appropriate */
    if (BSP_IS_HOLLYWOOD(hwver) || BSP_IS_HOLLYWOOD_ES1(hwver) || BSP_IS_BOLLYWOOD(hwver)) {
        reg |= HW_IOSTRCTRL0_SI(1);
    } else if (BSP_IS_LATTE(hwver)) {
        reg |= HW_IOSTRCTRL0_SI(2);
    } else return BSP_RVAL_UNKNOWN_HARDWARE_VERSION;

    *HW_IOSTRCTRL0 = reg;
    return BSP_RVAL_OK;
}

static BSP_RVAL bspSI_IOPower_read(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pReadData) {
    uint32_t reg;

    reg = *HW_IOPWRCTRL;
    if (reg & HW_IOPWRCTRL_SI(1) && reg & HW_IOPWRCTRL_SI(2)) {
        *(uint8_t*)pReadData = 1;
    } else {
        *(uint8_t*)pReadData = 0;
    }

    return BSP_RVAL_OK;
}

static BSP_RVAL bspSI_IOPower_write(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pWrittenData) {
    uint32_t reg;

    reg = *HW_IOPWRCTRL;
    reg &= ~HW_IOPWRCTRL_SI(HW_IOPWRCTRL_MASK);

    if (*(uint8_t*)pWrittenData != 0) {
        reg |= HW_IOPWRCTRL_SI(1) | HW_IOPWRCTRL_SI(2);
    }

    *HW_IOPWRCTRL = reg;
    return BSP_RVAL_OK;
}

static BSP_RVAL bspSI_IOPower_init(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pInitData, ulong size) {
    uint8_t enable = (uint8_t)pAttribute->data;

/*  Sanitise: enable must be 0 or 1 */
    if (enable) enable = 1;

    bspSI_IOPower_write(instance, pAttribute, &enable);
}

static BSP_RVAL bspSI_Subsystem_init(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pInitData, ulong size) {
    BSP_RVAL err;
    BSP_SYSTEM_CLOCK_INFO clockInfo;
    uint32_t reg;

    err = BSPGetClockInfo(&clockInfo); //.text:e6001920
    if (err) return err;

    reg = *HW_SICLKDIV;
    reg &= 0xffffff00;

    //TODO figure out the abysmal math here
}
static BSP_ATTRIBUTE bspSI_attributes[] = { //.data:e6042004
    {
        .pName = "IOStrength",
        .options = BSP_AO_INIT_GLOBAL,
        .permissions = BSP_PERMISSIONS_IOS,
        .initMethod = bspSI_IOStrength_init,
    },
    {
        .pName = "BaseAddress",
        .options = BSP_AO_QUERY_DATA_DIRECT,
        .permissions = BSP_PERMISSIONS_IOS,
        .data = 0x0d006c00,
        .dataSize = 4,
    },
    {
        .pName = "IOPower",
        .options = BSP_AO_QUERY_DATA_DIRECT | BSP_AO_INIT_GLOBAL,
        .permissions = BSP_PERMISSIONS_IOS,
        .data = 1,
        .dataSize = 1,
        .readMethod = bspSI_IOPower_read,
        .writeMethod = bspSI_IOPower_write,
        .initMethod = bspSI_IOPower_init,
    },
    {
        .pName = "Subsystem",
        .options = BSP_AO_INIT_GLOBAL,
        .permissions = BSP_PERMISSIONS_IOS,
        .initMethod = bspSI_Subsystem_init,
    },
};

static BSP_ENTITY bspSI_entity = { //.rodata:e60400d0
    .name = "SI",
    .version = 1,
    .instanceCount = 1,
    .options = BSP_EO_NONE;
    .pAttributes = {
        &bspSI_attributes,
    },
};

BSP_RVAL bspSIInstall() {
    return bspRegisterEntity(&bspSI_entity);
}
