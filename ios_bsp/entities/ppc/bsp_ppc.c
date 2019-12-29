#include "bsp_api.h"

static byte summary[0x10]; //.bss:e6047840
/* 3 arrays of 21 bytes each */
static byte coreproperties[3][21]; //.bss:e6047850
/* not 100% on what this does */
static short eeStuff = 0x0042; //.data:e6043e4c

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
        .dataSize = 0x10,
    }, {
        .pName = "CoreProperties",
        .options = BSP_AO_QUERY_DATA_INDIRECT,
        .permissions = BSP_PERMISSIONS_ALL,
        .data = coreproperties[0],
        .dataSize = 21,
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
        .dataSize = 0x10,
    }, {
        .pName = "CoreProperties",
        .options = BSP_AO_QUERY_DATA_INDIRECT,
        .permissions = BSP_PERMISSIONS_ALL,
        .data = coreproperties[1],
        .dataSize = 21,
    },
};

static BSP_ATTRIBUTE ppcCore2Attributes[] = { { //.data:e6044034
        .pName = "Summary",
        .options = BSP_AO_QUERY_DATA_INDIRECT,
        .permissions = BSP_PERMISSIONS_ALL,
        .data = summary,
        .dataSize = 0x10,
    }, {
        .pName = "CoreProperties",
        .options = BSP_AO_QUERY_DATA_INDIRECT,
        .permissions = BSP_PERMISSIONS_ALL,
        .data = coreproperties[2],
        .dataSize = 21,
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
            eeStuff = 0x0042;
        } else {
            eeStuff = 0x0010;
        }

        for (int core = 0; core < 3; core++) {
            ret |= sub_E6007AE0(core, coreproperties[core]);
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

        ret = sub_E6007AE0(0, coreproperties[0]);
        if (!ret) return ret;
        eeStuff = 0x0042;
        pPPCEntity = wood_ppc_entity;
    }

    if (BSP_IS_BOLLYWOOD(hwver) ||
        BSP_IS_HOLLYWOOD(hwver) ||
        BSP_IS_HOLLYWOOD_ES1(hwver)) {
        pPPCExecStop = bspPPCExecStop;
        pPPCExecStart = bspPPCExecStart;
    } else if (BSP_IS_LATTE(hwver)) {
        pPPCExecStop = bspPPCExecStopLatte;
        pPPCExecStart = bspPPCExecStartLatte;
    }

    return bspRegisterEntity(pE);
}
