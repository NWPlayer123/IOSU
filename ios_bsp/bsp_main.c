#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ios_api/ios.h"
#include "bsp_public_api.h"
#include "bsp_hwrevs.h"

typedef struct fd_t {
    bool active;
    uint32_t permissions;
} fd_t;

unsigned int devbsp_msgqueue[0x40]; //.bss:E6047000 - E60470FC inclusive
IOSMessageQueue devbsp_msgqueue_id; //.bss:E6047100
fd_t bsp_fdtable[0x20]; //.bss:E6047104 - E6047200 inclusive
int log_fd = -1; //.data:E6042000

/*  .text:E6000000
    Entry point. No obvious arguments/return values.
    Name taken from log output */
void main() {
/*  ==================================== initialisation bit */
    int ret, bsperr/*r9*/, ioserr/*r8*/;

    sub_E600E804();
    ret = IOS_CreateCrossProcessHeap(0x10000); //UND #0x250, .text:E600FA44
    if (ret < IOS_ERROR_OK) {
    /*  Not 100% sure if this is a generic print or an error reporter */
        bsp_print("BSP: IOS_CreateCrossProcessHeap ERROR, rval = %d\n", ret); //.text:E600EC28
        return;
    }

    memset(bsp_fdtable, 0, sizeof(bsp_fdtable)); //.text:E600EAB4

    devbsp_msgqueue_id = IOS_CreateMessageQueue(devbsp_msgqueue, 0x40); //UND #0xC0, .text:E600F97C
    if (devbsp_msgqueue_id < IOS_ERROR_OK) {
        ioserr = devbsp_msgqueue_id;
        bsperr = 0;
        goto report_err;
    }

    ret = IOS_RegisterResourceManager("/dev/bsp", devbsp_msgqueue_id); //UND #0x2C0, .text:E600FA7C
    if (ret != IOS_ERROR_OK) {
        ioserr = ret;
        bsperr = 0;
        goto report_err;
    }

    ret = IOS_DeviceAssosciate("/dev/bsp", 1); //UND #0x2D0, .text:E600FA84
    if (ret != IOS_ERROR_OK) {
        ioserr = ret;
        bsperr = 0;
        goto report_err;
    }

    ret = init_stuff(); //.text:E6000D78, see below
    if (ret) {
        ioserr = 0;
        bsperr = ret;
        goto report_err;
    }

    IOS_SetBSPReady(); //UND #0x5C0, .text:E600FBFC

/*  ==================================== /dev/bsp processing bit */
    struct IOSMessage* msg;
    IOSError result;
    unsigned int perms_u, perms_l;
    unsigned int* inPtr;
    unsigned int* outPtr;

    for (;;) {
        ret = IOS_ReceiveMessage(devbsp_msgqueue_id, &msg, 0); //UND #0x100, .text:E600F99C
        if (ret) {
            ioserr = 0;
            bsperr = 0;
            break; //aka goto report_err;
        }

        IOSMessageCommand cmd = __builtin_bswap32(msg->command);
        if (cmd == IOS_CLOSE) {
        /*  Grab client fd, check it's valid/active */
            int fd = __builtin_bswap32(msg->clientFD);
            if (fd > 0x1F || !bsp_fdtable[fd].active) {
                result = IOS_ERROR_NOEXISTS;
                goto reply;
            }

        /*  Free the fd */
            memset(&bsp_fdtable[fd], 0, sizeof(bsp_fdtable[fd]));

        } else if (cmd == IOS_IOCTL) {
            int fd = __builtin_bswap32(msg->clientFD);
            if (fd > 0x1F || !bsp_fdtable[fd].active) {
                result = IOS_ERROR_NOEXISTS;
                goto reply;
            }

        /*  ... these pointers never seem to be sanity-checked */
            uint32_t cmd = (uint32_t)__builtin_bswap32(msg->arg0);
            uint32_t* inPtr = (uint32_t*)__builtin_bswap32(msg->arg1);
            uint32_t* outPtr = (uint32_t*)__builtin_bswap32(msg->arg3);
            BSP_IOCTL_METHOD* args = (BSP_IOCTL_METHOD*)inPtr;

            switch (cmd) {
                case BSP_IOCTL_GET_ENTITY_VERSION: {
                    if (__builtin_bswap32(args->size) != 4) {
                        result = BSP_RVAL_SPECIFIED_SIZE_INVALID;
                    } else {
                        result = sub_E6000668(inPtr, outPtr);
                    }
                }
                case BSP_IOCTL_GET_HARDWARE_VERSION: {
                    if (__builtin_bswap32(args->size) != 4) {
                        result = BSP_RVAL_SPECIFIED_SIZE_INVALID;
                    } else {
                        result = bspMethodGetHardwareVersion(outPtr);
                    }
                }
                case BSP_IOCTL_GET_CONSOLE_MASK: {
                    if (__builtin_bswap32(args->size) != 4) {
                        result = BSP_RVAL_SPECIFIED_SIZE_INVALID;
                    } else {
                        result = sub_E600B1A4(outPtr);
                    }
                }
                case BSP_IOCTL_METHOD_QUERY: {
                    result = bspMethodQuery(args->entityName, __builtin_bswap32(args->instance), &args->attributeName, __builtin_bswap32(args->size), outPtr, bsp_fdtable[fd].permissions); //.text:e6000a90
                }
                case BSP_IOCTL_METHOD_READ: {
                    result = bspMethodRead(args->entityName, __builtin_bswap32(args->instance), &args->attributeName, __builtin_bswap32(args->size), outPtr, bsp_fdtable[fd].permissions); //.text:e6000a14
                }
                case BSP_IOCTL_METHOD_WRITE: {
                    result = bspMethodWrite(args->entityName, __builtin_bswap32(args->instance), &args->attributeName, __builtin_bswap32(args->size), &args->inData, bsp_fdtable[fd].permissions); //.text:e6000998
                }
                case BSP_IOCTL_METHOD_INIT: {
                    result = bspMethodInit(args->entityName, __builtin_bswap32(args->instance), &args->attributeName, __builtin_bswap32(args->size), &args->inData, bsp_fdtable[fd].permissions); //.text:e6000890
                }
                case BSP_IOCTL_METHOD_SHUTDOWN: {
                    result = bspMethodShutdown(args->entityName, __builtin_bswap32(args->instance), &args->attributeName, bsp_fdtable[fd].permissions); //.text:e6000830
                }
                case BSP_IOCTL_GET_CONSOLE_TYPE: {
                    if (__builtin_bswap32(args->size) != 4) {
                        result = BSP_RVAL_SPECIFIED_SIZE_INVALID;
                    } else {
                        result = sub_E600B270(outPtr);
                    }
                }
                default: {
                    result = IOS_ERROR_INVALID;
                    goto reply;
                }
            }
        } else if (cmd == IOS_OPEN) {
        /*  uint32_t perms_u = __builtin_bswap32(msg->arg3); unused? */
            uint32_t perms_l = __builtin_bswap32(msg->arg4);

            int fd;
            for (fd = 0; fd != 0x20; fd++) {
            /*  Find first unused fd */
                if (!bsp_fdtable[fd].active) {
                /*  Allocate it, noting permissions */
                    bsp_fdtable[fd].active = true;
                    bsp_fdtable[fd].permissions = perms_l;
                    break; //aka goto reply;
                }
            }
        /*  Result gets set to *something* here, but it's not clear what */
        } else {
            result = IOS_ERROR_INVALID;
            goto reply;
        }

reply:
        IOS_ResourceReply(msg, result); //UND #0x490, .text:E600FB64
    }

report_err:
#ifdef ACCURATE
    sub_E600E264(log_fd, "main", "bsp_main.c", 0x97, "main() is exiting, ioserr %d, bsp err 0x%x\n", ioserr, bsperr);
#else
    sub_E600E264(log_fd, __FUNCTION__, __FILE__, __LINE__, "main() is exiting, ioserr %d, bsp err 0x%x\n", ioserr, bsperr);
#endif
}

unsigned int init_stuff() {
    unsigned int ret /*r4|r5*/;
    ret = sub_E6000CC4();
    ret |= sub_E600580C(); //bspResetInstall
    ret |= sub_E6006BA4(); //bspRAMInstall
    ret |= sub_E6006A3C(); //bspDIInstall
    ret |= sub_E6007D5C(); //bspPPCInstall
    ret |= sub_E6004E68(); //bspSDIOInstall
    ret |= sub_E6003198(); //bspVIInstall
    ret |= sub_E6000F80(); //bspAIInstall
    ret |= sub_E6002800(); //bspUSBInstall
    ret |= sub_E60068CC(); //bspSIInstall
    ret |= sub_E6005D5C(); //bspCortadoInstall
    ret |= sub_E6008C0C(); //bspSATAInstall
    ret |= sub_E6009584(); //bspFLAInstall
    ret |= sub_E60095C4(); //bspSMCInstall
    ret |= sub_E60099CC(); //bspRTCInstall
    ret |= sub_E6009D68(); //bspCCRHInstall
    ret |= sub_E600A2B0(); //bspWIFIInstall
    ret |= sub_E60040D8(); //bspGFXInstall
    ret |= sub_E600A398(); //bspEEInstall
    ret |= sub_E600B0D8(); //bspDDRPerfInstall
    ret |= sub_E600B864(); //bspDISPLAYInstall
    return ret;
}

// now for hardware versioning: this bit doesn't make much sense yet

unsigned int bspEntityCount; //.bss:E6047204
BSP_ENTITY* bspEntityList[0x20]; //.bss:E6047208
BSP_HARDWARE_VERSION bspHardwareVersion; //.bss:E6047984
BC_CONFIG bspBoardConfig; //.bss:E604798C

/*  Some kind of Wood hardware init?
 *  .text:E6000CC4
 */
int sub_E6000CC4() {
    int error /*r4|r5*/;

    bspEntityCount = 0;
    memset(&bspEntityList, 0, sizeof(bspEntityList));

    log_fd = log_open("BSP", 1, 3); //.text:E604697C; name from log output

    *LT_RESETS_COMPAT |= 0xD0000; //acc. to WiiBrew, this is DDR2, I and DDR1
    *LT_EXICTRL = (*LT_EXICTRL & ~1) | 1; //enable EXI

    error  = InitASICVersion(&bspHardwareVersion); //.text:E600B5B4 - see below
    error |= bspGPIOInstall(); //see below

    bspHardwareVersion = 0; //are you kidding me, Nintendo?

    error |= bspMethodGetHardwareVersion(&bspHardwareVersion); //see below

    return error;
}

/*  Installs GPIO entity
 *  .text:e6006800
 */
int bspGPIOInstall() {
    int ret;
    BSP_HARDWARE_VERSION hwver;

    ret = bspMethodGetHardwareVersion(&hwver); //.text:E600B62C - see below
    if (ret != BSP_RVAL_OK) return ret;

    if ((hwver & 0xF0000000) == 0) {
        return BSP_RVAL_UNKNOWN_HARDWARE_VERSION;
    }

    return bspRegisterEntity(&gpio_latte_entity);
}

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

/*  Something to do with getting the Wood version?
*   .text:e600b5b4
*/
int InitASICVersion(unsigned int* woodver) {
    int ret;

    *woodver = bspHardwareVersion;
    if (bspHardwareVersion) return 0;

    ret = determineWoodBasedHardwareVersion(woodver, false); //.text:E600B41C - see below
    if (ret) return ret;

    if (*woodver == BSP_HARDWARE_VERSION_BOLLYWOOD_PROD_FOR_WII) {
        if (GetLatteHardwareVersion(woodver) == 0) {
            *woodver = *woodver & 0xFFFF0000 | 0x11;
         /* This appears to force a EV_Y board? */
        }
    }
    return 0;
}

unsigned short word_E6047988; //.bss:E6047988. IDA reckons it's a byte.

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
            *version = BSP_HARDWARE_VERSION_CORTADO;
            if (!fullCheck) return BSP_RVAL_OK;

            unsigned int isCortado;
            ret = CheckCortado(&isCortado); //.text:E6005818
            if (ret) return BSP_RVAL_OK;

            if (!isCortado) {
                *version = BSP_HARDWARE_VERSION_HOLLYWOOD_PROD_FOR_WII;
                return BSP_RVAL_OK;
            }

            sub_E6007C5C(&word_E6047988);
            *version = BSP_HARDWARE_VERSION_CORTADO;
            if (word_E6047988 == 0x7001) {
                *version = BSP_HARDWARE_VERSION_CORTADO_ESPRESSO;
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
