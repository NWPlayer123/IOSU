#include "bsp_api.h"

static BSP_PPC_SUMMARY summary; //.bss:e6047840
static BSP_PPC_CORE_PROPERTIES coreproperties[3]; //.bss:e6047850
/* location of the PVR in SEEPROM ("ee") */
#define BSP_EE_PVR_HLYWD 0x0042
#define BSP_EE_PVR_LATTE 0x0010
static short eePVRAddr = BSP_EE_PVR_HLYWD; //.data:e6043e4c

static BSP_RVAL(*pPPCExecStop)(void); //.bss:e6047838
static BSP_RVAL(*pPPCExecStart)(void); //.bss:e604783c

static BSP_ATTRIBUTE ppcCore0Attributes[] = { { //.data:e6043e50
        .pName = "Exe",
        .permissions = BSP_PERMISSIONS_IOS_SUPV,
        .dataSize = 1,
        .writeMethod = bspPPC_Exec,
    }, {
        .pName = "Clock",
        .permissions = BSP_PERMISSIONS_IOS_SUPV,
        .dataSize = 1,
        .writeMethod = bspPPC_Clock,
    }, {
        .pName = "EXIRegBoot",
        .permissions = BSP_PERMISSIONS_IOS_SUPV,
        .dataSize = 0x48,
        .writeMethod = bspPPC_EXIRegBoot,
    }, {
        .pName = "Summary",
        .options = BSP_AO_QUERY_DATA_INDIRECT,
        .permissions = BSP_PERMISSIONS_ALL,
        .data = summary,
        .dataSize = sizeof(BSP_PPC_SUMMARY),
    }, {
        .pName = "CoreProperties",
        .options = BSP_AO_QUERY_DATA_INDIRECT,
        .permissions = BSP_PERMISSIONS_ALL,
        .data = coreproperties[0],
        .dataSize = sizeof(BSP_PPC_CORE_PROPERTIES),
    }, {
        .pName = "PVR",
        .permissions = BSP_PERMISSIONS_ALL,
        .dataSize = 4,
        .readMethod = bspPPC_ReadPVR,
        .writeMethod = bspPPC_WritePVR,
    }, {
        .pName = "60XeDataStreaming",
        .permissions = BSP_PERMISSIONS_ALL,
        .dataSize = 1,
        .readMethod = bspPPC_Read60XeDataStreaming,
        .writeMethod = bspPPC_Write60XeDataStreaming,
    }
};

static BSP_ATTRIBUTE ppcCore1Attributes[] = { { //.data:e6043fb0
        .pName = "Summary",
        .options = BSP_AO_QUERY_DATA_INDIRECT,
        .permissions = BSP_PERMISSIONS_ALL,
        .data = summary,
        .dataSize = sizeof(BSP_PPC_SUMMARY),
    }, {
        .pName = "CoreProperties",
        .options = BSP_AO_QUERY_DATA_INDIRECT,
        .permissions = BSP_PERMISSIONS_ALL,
        .data = coreproperties[1],
        .dataSize = sizeof(BSP_PPC_CORE_PROPERTIES),
    },
};

static BSP_ATTRIBUTE ppcCore2Attributes[] = { { //.data:e6044034
        .pName = "Summary",
        .options = BSP_AO_QUERY_DATA_INDIRECT,
        .permissions = BSP_PERMISSIONS_ALL,
        .data = summary,
        .dataSize = sizeof(BSP_PPC_SUMMARY),
    }, {
        .pName = "CoreProperties",
        .options = BSP_AO_QUERY_DATA_INDIRECT,
        .permissions = BSP_PERMISSIONS_ALL,
        .data = coreproperties[2],
        .dataSize = sizeof(BSP_PPC_CORE_PROPERTIES),
    },
};

static BSP_ENTITY wood_ppc_entity = { //.rodata:e6040ad0
    .name = "PPC",
    .version = 1,
    .instanceCount = 1,
    .options = BSP_EO_NONE;
    .pAttributes = {
        &ppcCore0Attributes,
    },
};
static BSP_ENTITY latte_ppc_entity = { //.rodata:e6040ab4
    .name = "PPC",
    .version = 1,
    .instanceCount = 3,
    .options = BSP_EO_NONE;
    .pAttributes = {
        &ppcCore0Attributes,
        &ppcCore1Attributes,
        &ppcCore2Attributes,
    },
};

BSP_RVAL bspPPCInstall() {
    BSP_HARDWARE_VERSION hwver;
    BSP_ENTITY* pPPCEntity;
    BSP_RVAL ret;

    memset(summary, 0, sizeof(summary));
    memset(coreproperties, 0, sizeof(coreproperties));

    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret != BSP_RVAL_OK) return ret;

    ret = bspPPCFillOutSummary(summary); //.text:e60079c4
    if (ret != BSP_RVAL_OK) return ret;

/*  TODO the control flow here might not be 100% accurate */
    if (hwver == BSP_HARDWARE_VERSION_HOLLYWOOD_CORTADO_ESPRESSO ||
        BSP_IS_LATTE(hwver)) {
        if (hwver == BSP_HARDWARE_VERSION_HOLLYWOOD_CORTADO_ESPRESSO) {
        /*  Use Wii-style SEEPROM format */
            eePVRAddr = BSP_EE_PVR_HLYWD;
        } else {
        /*  Use WiiU-style SEEPROM format */
            eePVRAddr = BSP_EE_PVR_LATTE;
        }

        for (int core = 0; core < 3; core++) {
            ret |= bspPPCFillOutCoreProperties(core, coreproperties[core]);
        }
        if (!ret) return ret;
        pPPCEntity = latte_ppc_entity;
    } else {
    /*  This is a Wii or other machine with a single-core PowerPC */
        if (!BSP_IS_BOLLYWOOD(hwver) &&
            !BSP_IS_HOLLYWOOD(hwver) &&
            !BSP_IS_HOLLYWOOD_ES1(hwver)) {
            return BSP_RVAL_UNKNOWN_HARDWARE_VERSION;
        }

        ret = bspPPCFillOutCoreProperties(0, coreproperties[0]);
        if (!ret) return ret;
    /*  Use Wii-style SEEPROM format */
        eePVRAddr = BSP_EE_PVR_HLYWD;
        pPPCEntity = wood_ppc_entity;
    }

    if (BSP_IS_BOLLYWOOD(hwver) ||
        BSP_IS_HOLLYWOOD(hwver) ||
        BSP_IS_HOLLYWOOD_ES1(hwver)) {
        pPPCExecStop = bspPPCExecStop; //.text:e6008154
        pPPCExecStart = bspPPCExecStart; //.text:e600811c
    } else if (BSP_IS_LATTE(hwver)) {
        pPPCExecStop = bspPPCExecStopLatte; //.text:e6008014
        pPPCExecStart = bspPPCExecStartLatte; //.text:e6007ff0
    }

    return bspRegisterEntity(pE);
}

//.text:e60079c4
BSP_RVAL bspPPCFillOutSummary(BSP_PPC_SUMMARY* summary) {
    BSP_RVAL ret;
    BSP_HARDWARE_VERSION hwver;

    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret != BSP_RVAL_OK) return ret;

    ret = bspGetSystemClockInfo(&summary->clock60x); //.text:e6001920
    if (ret != BSP_RVAL_OK) return ret;

    if (BSP_IS_LATTE(hwver)) {
        summary->numberOfCores = 3;
    } else {
        if (!BSP_IS_BOLLYWOOD(hwver) &&
            !BSP_IS_HOLLYWOOD(hwver) &&
            !BSP_IS_HOLLYWOOD_ES1(hwver)) {
            return BSP_RVAL_UNKNOWN_HARDWARE_VERSION;
        }
        if (hwver == BSP_HARDWARE_VERSION_HOLLYWOOD_CORTADO_ESPRESSO) {
            summary->numberOfCores = 3;
        } else {
            summary->numberOfCores = 1;
        }
    }

    for (int i = 0; i < summary->numberOfCores; i++) {
        summary->activeCoreBitmap |= 1 << i;
    }

    return BSP_RVAL_OK;
}

//.text:e6007ae0
BSP_RVAL bspPPCFillOutCoreProperties(uint ndx, BSP_PPC_CORE_PROPERTIES* core) {
    BSP_RVAL ret;
    BSP_HARDWARE_VERSION hwver;
    BSP_PPC_SUMMARY ppc;

    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret != BSP_RVAL_OK) return ret;

    ret = bspPPCFillOutSummary(&ppc);
    if (ret != BSP_RVAL_OK) return ret;

    memset(core, 0, sizeof(*core));
    if (ndx < ppc.numberOfCores) {
        core->coreActive = 1;

        if (BSP_IS_LATTE(hwver) ||
            hwver == BSP_HARDWARE_VERSION_HOLLYWOOD_CORTADO_ESPRESSO) {

            if (ndx == 1) {
                core->l2Size = 0x800;
            } else {
                core->l2Size = 0x200;
            }
            core->l2LineSize = 0x40;
            core->l2SectorSize = 0x20;
            core->l2FetchSize = 0x20;
            core->l2SetAssociativity = 4;
        } else {
            if (!BSP_IS_BOLLYWOOD(hwver) &&
                !BSP_IS_HOLLYWOOD(hwver) &&
                !BSP_IS_HOLLYWOOD_ES1(hwver)) {
                return BSP_RVAL_UNKNOWN_HARDWARE_VERSION;
            }

            core->l2Size = 0x100;
            core->l2LineSize = 0x40;
            core->l2SectorSize = 0x20;
            core->l2FetchSize = 0x20;
            core->l2SetAssociativity = 2;
        }
    }

    return BSP_RVAL_OK;
}

//.text:e6007c5c
BSP_RVAL bspPPCGetPVR(BSP_PPC_PVR* pvr) {
    IOSError ret;

    ret = bspEepromRead(eePVRAddr, 1, &pvr->version);
    if (ret != IOS_ERROR_OK) return BSP_RVAL_DEVICE_ERROR;

    ret = bspEepromRead(eePVRAddr + 1, 1, &pvr->revision);
    if (ret != IOS_ERROR_OK) return BSP_RVAL_DEVICE_ERROR;

    return BSP_RVAL_OK;
}

//.text:e6007cb0
BSP_RVAL bspPPC_ReadPVR(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pReadData) {
    BSP_PPC_PVR* pvr = (BSP_PPC_PVR*)pReadData;
    return bspPPCGetPVR(pvr);
}

//.text:e6007d38
BSP_RVAL bspPPC_WritePVR(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pWrittenData) {
    BSP_PPC_PVR* pvr = (BSP_PPC_PVR*)pWrittenData;
    return bspPPCSetPVR(*pvr);
}

//.text:e6007808
uint32_t bspPPCEnableEXI() {
    *HW_AIP_PROT |= HW_AIP_PROT_ENAHBIOPI;
    return *HW_AIP_PROT;
}

/* I feel I may have start and stop backwards */

//.text:e6007ff0
BSP_RVAL bspPPCExecStartLatte() {
    *HW_RSTB &= ~(HW_RSTB_CPU | HW_RSTB_PI);
    BSPSleep(0xf);
    return BSP_RVAL_OK;
}

//.text:e600811c
BSP_RVAL bspPPCExecStart() {
    uint32_t rstb = *HW_RSTB;
    *HW_RSTB = rstb & ~(HW_RSTB_CPU | HW_SRSTB_CPU);
    BSPSleep(0xf);
    uint32_t srstb = *HW_RSTB;
    *HW_RSTB = srstb & ~HW_SRSTB_CPU | rstb & HW_SRSTB_CPU;
    return BSP_RVAL_OK;
}

//.text:e6008154
BSP_RVAL bspPPCExecStop() {
    *HW_AIP_PROT &= ~HW_AIP_PROT_ENAHBIOPI;

    uint32_t rstb = *HW_RSTB & ~(HW_RSTB_CPU | HW_SRSTB_CPU);
    *HW_RSTB = rstb;
    BSPSleep(0xf);
    *HW_RSTB = rstb | HW_RSTB_CPU;
    BSPSleep(0x96);
    *HW_RSTB = rstb | HW_RSTB_CPU | HW_SRSTB_CPU;

/*  Wait for PowerPC to set IPC flag X2, ack, and update the PVR in SEEPROM. */
    for (int i = 0; i < 100; i++) {
        uint32_t armctrl = *HW_IPC_ARMCTRL;
        if (armctrl & HW_IPC_ARMCTRL_X2) {
        /*  Clear X2.
            Note that the original code is more like (a & X2 | X2), which as far
            as I can tell is equivalent? Maybe it's an ARM thing. */
            *HW_IPC_ARMCTRL = armctrl | HW_IPC_ARMCTRL_X2;
        /*  Very clear X2, and set Y2 */
            *HW_IPC_ARMCTRL = armctrl | (HW_IPC_ARMCTRL_X2 | HW_IPC_ARMCTRL_Y2);

            bspPPCEnableEXI();

        /*  Check EXI boot comms area for PowerPC PVR - if it's different to
            expected, update the SEEPROM. Yes, really. */
            BSP_PPC_PVR pvr;
            bspPPCGetPVR(&pvr);
        /*  What do you mean "this doesn't compile?" */
            if (HW_EXI_BOOT[15] != pvr) {
                bspPPCSetPVR(HW_EXI_BOOT[15]); //.text:e6007cb8
            }

            return BSP_RVAL_OK;
        }
        BSPSleep(10000);
    }

    bspPPCEnableEXI();
    return BSP_RVAL_DEVICE_ERROR;
}

//.text:e600794c
BSP_RVAL bspPPC_Exec(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pWrittenData) {
    BSP_RVAL ret;
    BSP_HARDWARE_VERSION hwver = BSP_HARDWARE_VERSION_UNKNOWN;
    bspMethodGetHardwareVersion(&hwver);

    uint8_t cmd = *(uint8_t*)pWrittenData;

    if (cmd == 1) {
        if (!pPPCExecStop) return BSP_RVAL_UNSUPPORTED_METHOD;
        return pPPCExecStop();
    }
    if (cmd == 2) {
        if (!pPPCExecStart) return BSP_RVAL_UNSUPPORTED_METHOD;
        return pPPCExecStart();
    }

    return BSP_RVAL_INVALID_PARAMETER;
}

BSP_RVAL bspPPC_Read60XeDataStreaming(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pReadData) {
    BSP_RVAL ret;
    BSP_HARDWARE_VERSION hwver;

    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret != BSP_RVAL_OK) return ret;

    if (!BSP_IS_LATTE(hwver)) return BSP_RVAL_UNSUPPORTED_METHOD;

    int8_t streaming = !(*LT_60XE_CFG & LT_60XE_CFG_STREAMING);
    *(int8_t*)pReadData = streaming;

    return BSP_RVAL_OK;
}

BSP_RVAL bspPPC_Write60XeDataStreaming(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pWrittenData) {
    BSP_RVAL ret;
    BSP_HARDWARE_VERSION hwver;

    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret != BSP_RVAL_OK) return ret;

    if (!BSP_IS_LATTE(hwver)) return BSP_RVAL_UNSUPPORTED_METHOD;

    uint32_t reg = *LT_60XE_CFG;
    *LT_60XE_CFG = reg | LT_60XE_CFG_LATCH;
    BSPSleep(10);

    uint8_t cmd = *(uint8_t*)pWrittenData;
    uint32_t streaming = (cmd == 1) ? 0 : LT_60XE_CFG_STREAMING;

    *LT_60XE_CFG = reg | streaming | LT_60XE_CFG_LATCH;
    *LT_60XE_CFG = (reg | streaming) & ~LT_60XE_CFG_LATCH;

    return BSP_RVAL_OK;
}
