#include "bsp_api.h"

static BSP_PPC_SUMMARY summary; //.bss:e6047840
static BSP_PPC_CORE_PROPERTIES coreproperties[3]; //.bss:e6047850
/* location of the PVR in SEEPROM ("ee") */
#define BSP_EE_PVR_HLYWD 0x0042
#define BSP_EE_PVR_LATTE 0x0010
static short eePVRAddr = BSP_EE_PVR_HLYWD; //.data:e6043e4c

static void(*pPPCExecStop)(void); //.bss:e6047838
static void(*pPPCExecStart)(void); //.bss:e604783c



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
