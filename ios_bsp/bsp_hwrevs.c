#include "bsp_hwrevs.h"

/*  Something to do with getting the hardware version?
*   .text:e600b62c
*/
BSP_RVAL bspMethodGetHardwareVersion(BSP_HARDWARE_VERSION *version) {
    int ret;

    *version = bspHardwareVersion;
    if (bspHardwareVersion) return BSP_RVAL_OK;

    ret = determineWoodBasedHardwareVersion(version, true); //.text:E600B41C - see below
    if (ret != BSP_RVAL_OK) return ret;

    if (*version == BSP_HARDWARE_VERSION_BOLLYWOOD_PROD_FOR_WII) {
        ret = determineLatteBasedHardwareVersion(version);
        if (ret != BSP_RVAL_OK) {
            bspHardwareVersion = *version;
            return BSP_RVAL_OK;
        }

        *version &= 0xFFFF0000;

        ret = bspMethodReadEEBoardConfig(&bspBoardConfig);
        if (ret != BSP_RVAL_OK) {
            *version |= BSP_VARIANT_EV_Y;
        } else {
            switch (bspBoardConfig.boardType) {
                case 0x4346: //"CF"
                    *version |= BSP_VARIANT_CAFE;
                    break;
                case 0x4354: //"CT"
                    *version |= BSP_VARIANT_CAT;
                    break;
                case 0x4556: //"EV"
                    *version |= BSP_VARIANT_EV;
                    break;
                case 0x4944: //"ID"
                    *version |= BSP_VARIANT_ID; //not in c2w symbols
                    break;
                case 0x4948: //"IH"
                    *version |= BSP_VARIANT_IH; //not in c2w symbols
                    break;
            }
        }
    }

    bspHardwareVersion = *version;
    return BSP_RVAL_OK;
}

static BSP_PPC_PVR pvr; //.bss:e6047988

/*  .text:E600B41C
    Decodes LT_ASICREV_ACR aka HW_CHIPREVID; placing the result in woodver.
    Doesn't seem to be an ASICREV_ACR revision for HOLLYWOOD_PROD_FOR_WII or
    HOLLYWOOD_CORTADO_ESPRESSO, this is determined from other factors */
BSP_RVAL determineWoodBasedHardwareVersion(BSP_HARDWARE_VERSION* version, bool fullCheck) {
    int ret;

    switch (*HW_CHIPREVID & 0xFF /*VERLO: revision*/) {
        case 0x00: {
            *version = BSP_HARDWARE_VERSION_HOLLYWOOD_ENG_SAMPLE_1;
            return BSP_RVAL_OK;
        }
        case 0x10: {
            *version = BSP_HARDWARE_VERSION_HOLLYWOOD_ENG_SAMPLE_2;
            return BSP_RVAL_OK;
        }
        case 0x11: {
            *version = BSP_HARDWARE_VERSION_HOLLYWOOD_CORTADO;
            if (!fullCheck) return BSP_RVAL_OK;

            unsigned int isCortado;
            ret = BSPCheckCortado(&isCortado); //.text:E6005818
            if (ret) return BSP_RVAL_OK;

            if (!isCortado) {
                *version = BSP_HARDWARE_VERSION_HOLLYWOOD_PROD_FOR_WII;
                return BSP_RVAL_OK;
            }

            bspPPCGetPVR(&pvr);
            *version = BSP_HARDWARE_VERSION_HOLLYWOOD_CORTADO;
            if (pvr.version == 0x7001) {
                *version = BSP_HARDWARE_VERSION_HOLLYWOOD_CORTADO_ESPRESSO;
            }
            return BSP_RVAL_OK;
        }
        case 0x20: {
            *version = BSP_HARDWARE_VERSION_BOLLYWOOD;
            return BSP_RVAL_OK;
        }
        case 0x21: {
            *version = BSP_HARDWARE_VERSION_BOLLYWOOD_PROD_FOR_WII;
            return BSP_RVAL_OK;
        }
        default: {
            *version = BSP_HARDWARE_VERSION_UNKNOWN;
            return BSP_RVAL_UNKNOWN_HARDWARE_VERSION;
        }
    }
}

//.text:E600B328
BSP_RVAL determineLatteBasedHardwareVersion(BSP_HARDWARE_VERSION* version) {
    int ret;

    uint32_t lt_chiprev = *LT_CHIPREVID;
    if (lt_chiprev & 0xFFFF0000 != 0xCAFE0000) {
        return BSP_RVAL_UNKNOWN_HARDWARE_VERSION;
    }

    *version &= 0x000FFFFF;

    switch (lt_chiprev & 0xFF) /* VERLO */ {
        case 0x10:
            *version |= 0x21100000; //Latte A11
            return BSP_RVAL_OK;
        case 0x18:
            *version |= 0x21200000; //Latte A12
            return BSP_RVAL_OK;
        case 0x21:
            *version |= 0x22100000; //Latte A2x
            return BSP_RVAL_OK;
        case 0x30:
            *version |= 0x23100000; //Latte A3x
            return BSP_RVAL_OK;
        case 0x40:
            *version |= 0x24100000; //Latte A4x
            return BSP_RVAL_OK;
        case 0x50:
            *version |= 0x25100000; //Latte A5x
            return BSP_RVAL_OK;
        default:
            *version |= 0x26100000; //Latte B1x
            return BSP_RVAL_OK;
    }
}
