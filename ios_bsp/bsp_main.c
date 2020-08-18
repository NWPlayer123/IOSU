#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <ios_api/ios.h>
#include <bc.h>
#include <log.h>

#include "latte/latte.h"

#include <bsp_entity.h>
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

void hello();
BSP_RVAL bspInstall();
BSP_RVAL InitASICVersion(BSP_HARDWARE_VERSION* version);

/*  .text:E6000000
    Entry point. No obvious arguments/return values.
    Name taken from log output */
void main() {
/*  ==================================== initialisation bit */
    int ret, bsperr/*r9*/, ioserr/*r8*/;

    hello();
    ret = IOS_CreateCrossProcessHeap(0x10000); //UND #0x250, .text:E600FA44
    if (ret < IOS_ERROR_OK) {
    /*  Not 100% sure if this is a generic print or an error reporter */
        log_debug_printf("BSP: IOS_CreateCrossProcessHeap ERROR, rval = %d\n", ret); //.text:E600EC28
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

    ret = bspInstall(); //.text:E6000D78, see below
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
                    result = bspMethodQuery(args->entityName, __builtin_bswap32(args->instance), args->attributeName, __builtin_bswap32(args->size), outPtr, bsp_fdtable[fd].permissions); //.text:e6000a90
                }
                case BSP_IOCTL_METHOD_READ: {
                    result = bspMethodRead(args->entityName, __builtin_bswap32(args->instance), args->attributeName, __builtin_bswap32(args->size), outPtr, bsp_fdtable[fd].permissions); //.text:e6000a14
                }
                case BSP_IOCTL_METHOD_WRITE: {
                    result = bspMethodWrite(args->entityName, __builtin_bswap32(args->instance), args->attributeName, __builtin_bswap32(args->size), &args->inData, bsp_fdtable[fd].permissions); //.text:e6000998
                }
                case BSP_IOCTL_METHOD_INIT: {
                    result = bspMethodInit(args->entityName, __builtin_bswap32(args->instance), args->attributeName, __builtin_bswap32(args->size), &args->inData, bsp_fdtable[fd].permissions); //.text:e6000890
                }
                case BSP_IOCTL_METHOD_SHUTDOWN: {
                    result = bspMethodShutdown(args->entityName, __builtin_bswap32(args->instance), args->attributeName, bsp_fdtable[fd].permissions); //.text:e6000830
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

//.text:e600e804
void hello() {
    IOSProcessId process;
    char name[6];
    char build_date[18];

    memcpy(name,"error",6);
#ifdef ACCURATE
    int utilisation = 37;
    memcpy(build_date,"07/28/15 19:23:58",0x12);
#else
    #error Not implemented, use -DACCURATE
    //int utilisation = 37;
    //memcpy(build_date,"07/28/15 19:23:58",0x12);
#endif

    process = IOS_GetCurrentProcessId();
    IOS_GetProcessName(process,name);
    log_debug_printf("%s: Built %s, Image Utilization %d%%.\n",name,build_date,0x25);
    return;
}

//.text:e6000d78
BSP_RVAL bspInstall() {
    BSP_RVAL ret /*r4|r5*/;
    ret = sub_E6000CC4();
    ret |= bspResetInstall(); //sub_E600580C
    ret |= bspRAMInstall(); //.text:E6006BA4
    ret |= bspDIInstall(); //.text:E6006A3C
    ret |= bspPPCInstall(); //.text:E6007D5C
    ret |= bspSDIOInstall(); //.text:E6004E68
    ret |= bspVIInstall(); //.text:E6003198
    ret |= bspAIInstall(); //.text:E6000F80
    ret |= bspUSBInstall(); //.text:E6002800
    ret |= bspSIInstall(); //.text:E60068CC
    ret |= bspCortadoInstall(); //.text:E6005D5C
    ret |= bspSATAInstall(); //.text:E6008C0C
    ret |= bspFLAInstall(); //.text:E6009584
    ret |= bspSMCInstall(); //.text:E60095C4
    ret |= bspRTCInstall(); //.text:E60099CC
    ret |= bspCCRHInstall(); //.text:E6009D68
    ret |= bspWIFIInstall(); //.text:E600A2B0
    ret |= bspGFXInstall(); //.text:E60040D8
    ret |= bspEEInstall(); //.text:E600A398
    ret |= bspDDRPerfInstall(); //.text:E600B0D8
    ret |= bspDISPLAYInstall(); //.text:E600B864
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
int bspInitEssential() {
    int error /*r4|r5*/;

    bspEntityCount = 0;
    memset(&bspEntityList, 0, sizeof(bspEntityList));

    log_fd = log_open("BSP", 1, 3); //.text:E604697C; name from log output

    *HW_RSTB |= HW_RSTB_IOEXI | HW_RSTB_IOMEM | HW_RSTB_IOPI;
    *HW_AIP_PROT |= HW_AIP_PROT_ENAHBIOPI; //enable EXI

    error  = InitASICVersion(&bspHardwareVersion); //.text:E600B5B4 - see below
    error |= bspGPIOInstall(); //see below

    bspHardwareVersion = 0; //are you kidding me, Nintendo?

    error |= bspMethodGetHardwareVersion(&bspHardwareVersion); //see below

    return error;
}

// .text:e600b5b4
BSP_RVAL InitASICVersion(BSP_HARDWARE_VERSION* version) {
    int ret;

    *version = bspHardwareVersion;
    if (bspHardwareVersion) return 0;

    ret = determineWoodBasedHardwareVersion(version, false); //.text:E600B41C - see below
    if (ret) return ret;

    if (*version == BSP_HARDWARE_VERSION_BOLLYWOOD_PROD_FOR_WII) {
        if (determineLatteBasedHardwareVersion(version) == 0) {
            *version = *version & 0xFFFF0000 | 0x11;
         /* This appears to force a EV_Y board? */
        }
    }
    return 0;
}

BSP_SYSTEM_CLOCK_INFO BSPDefaultClockdata = {0,0}; //.rodata:e6040263

//.text:e6001920
BSP_RVAL bspGetSystemClockInfo(BSP_SYSTEM_CLOCK_INFO* clock) {
    BSP_RVAL ret;
    memcpy(clock, &BSPDefaultClockdata, sizeof(*clock));

    BSP_HARDWARE_VERSION hwver;
    ret = bspMethodGetHardwareVersion(&hwver);
    if (ret != BSP_RVAL_OK) return ret;

    if (BSP_IS_HOLLYWOOD(hwver) ||
        BSP_IS_HOLLYWOOD_ES1(hwver)) {
        if (*HW_CLOCKS & HW_CLOCKS_SPEED) {
            clock->systemClockFrequency = 162000000;
        } else {
            clock->systemClockFrequency = 243000000;
        }
        if (hwver == BSP_HARDWARE_VERSION_HOLLYWOOD_CORTADO_ESPRESSO ||
            hwver == BSP_HARDWARE_VERSION_HOLLYWOOD_CORTADO) {
            clock->systemClockFrequency = 167000000;
        }
    } else if (BSP_IS_LATTE(hwver)) {
        //inline:e60011a0
        if (bspBoardConfig.version > 2) {
            if (bspBoardConfig.sysPllSpeed == 0xf0) {
                clock->systemClockFrequency = 239625000;
            } else if (bspBoardConfig.sysPllSpeed == 0xf8) {
                clock->systemClockFrequency = 248625000;
            }
        } else {
            clock->systemClockFrequency = 243000000;
        }
        //finish inline
        if (*LT_PLLSYS & LT_PLLSYS_HALFSPEED) {
            clock->systemClockFrequency /= 2;
        }
    }
    clock->timerFrequency = clock->systemClockFrequency / 128;
    return BSP_RVAL_OK;
}
