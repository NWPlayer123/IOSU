#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ios_api/ios.h"
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

        /*  There's a cmd-- here. I've offset the case statements to account
            for this.
            These ioctls obviously operate on some kind of struct, though I've
            no idea what it might look like */
            switch (cmd) {
                case 0x1: {
                    if (__builtin_bswap32(inPtr[0x11]/*aka inPtr+0x44*/) != 4) {
                        result = 0x80;
                    } else {
                        result = sub_E6000668(inPtr, outPtr);
                    }
                }
                case 0x2: {
                    if (__builtin_bswap32(inPtr[0x11]/*inPtr+0x44*/) != 4) {
                        result = 0x80;
                    } else {
                        result = sub_E600B62C(outPtr);
                    }
                }
                case 0x3: {
                    if (__builtin_bswap32(inPtr[0x11]/*+0x44*/) != 4) {
                        result = 0x80;
                    } else {
                        result = sub_E600B1A4(outPtr);
                    }
                }
                case 0x4: {
                    result = sub_E6000A90(inPtr, __builtin_bswap32(inPtr[8]/*+0x20*/), &(inPtr[9]/*+0x24*/), __builtin_bswap32(inPtr[0x11]/*+0x44*/), /*stack args, unsure of order*/ outPtr, bsp_fdtable[fd].permissions);
                }
                case 0x5: {
                    result = sub_E6000A14(inPtr, __builtin_bswap32(inPtr[8]/*+0x20*/), &(inPtr[9]/*+0x24*/), __builtin_bswap32(inPtr[0x11]/*+0x44*/), /*stack args, unsure of order*/ outPtr, bsp_fdtable[fd].permissions);
                }
                case 0x6: {
                    result = sub_E6000998(inPtr, __builtin_bswap32(inPtr[8]/*+0x20*/), &(inPtr[9]/*+0x24*/), __builtin_bswap32(inPtr[0x11]/*+0x44*/), &(inPtr[0x12]/*+0x48*/), bsp_fdtable[fd].permissions);
                }
                case 0x7: {
                    result = sub_E6000890(inPtr, __builtin_bswap32(inPtr[8]/*+0x20*/), &(inPtr[9]/*+0x24*/), __builtin_bswap32(inPtr[0x11]/*+0x44*/), &(inPtr[0x12]/*+0x48*/), bsp_fdtable[fd].permissions);
                }
                case 0x8: {
                    result = sub_E6000830(inPtr, __builtin_bswap32(inPtr[8]/*+0x20*/), &(inPtr[9]/*+0x24*/), bsp_fdtable[fd].permissions);
                }
                case 0x9: {
                    if (__builtin_bswap32(inPtr[0x11]/*+0x44*/) != 4) {
                        result = 0x80;
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
    ret |= sub_E600580C();
    ret |= sub_E6006BA4();
    ret |= sub_E6006A3C();
    ret |= sub_E6007D5C();
    ret |= sub_E6004E68();
    ret |= sub_E6003198();
    ret |= sub_E6000F80();
    ret |= sub_E6002800();
    ret |= sub_E60068CC();
    ret |= sub_E6005D5C();
    ret |= sub_E6008C0C();
    ret |= sub_E6009584();
    ret |= sub_E60095C4();
    ret |= sub_E60099CC();
    ret |= sub_E6009D68();
    ret |= sub_E600A2B0();
    ret |= sub_E60040D8();
    ret |= sub_E600A398();
    ret |= sub_E600B0D8();
    ret |= sub_E600B864();
    return ret;
}

// now for hardware versioning: this bit doesn't make much sense yet

unsigned int dword_E6047204; //.bss:E6047204
unsigned int arr_E6047208[0x80]; //.bss:E6047208
unsigned int wood_hardware_ver; //.bss:E6047984

/*  Some kind of Wood hardware init?
*/
int sub_E6000CC4() {
    int error /*r4|r5*/;

    dword_E6047204 = 0;
    memset(&arr_E6047208, 0, 0x80);

    log_fd = log_open("BSP", 1, 3); //.text:E604697C; name from log output

    *LT_RESETS_COMPAT |= 0xD0000; //acc. to WiiBrew, this is DDR2, I and DDR1
    *LT_EXICTRL = (*LT_EXICTRL & ~1) | 1; //enable EXI

    error  = InitASICVersion(&wood_hardware_ver); //.text:E600B5B4 - see below
    error |= sub_E6006800(); //see below

    wood_hardware_ver = 0; //are you kidding me, Nintendo?

    error |= sub_E600B62C(&wood_hardware_ver); //see below
}

int sub_E6006800() {
    int ret;
    unsigned int woodver;

    ret = sub_E600B62C(&woodver); //.text:E600B62C - see below
    if (ret == 0) {
        if ((woodver & 0xF0000000) != 0) {
            return sub_E6000B60(/* &something */);
            //something: &"GPIO", 1, 1, 1, &&"IOStrength", 0x90000000, 0, 1
            //TODO probably a BSP entity
        }
        return 0x800;
    }
    //TODO I'm sick of hardware versioning
}

/*  Something to do with getting the hardware version?
*/
int sub_E600B62C(unsigned int *woodver) {
    int ret;

    *woodver = wood_hardware_ver;
    if (!wood_hardware_ver) return 0;

    ret = GetWoodHardwareVersion(woodver, true); //.text:E600B41C - see below
    if (ret) return ret;

    if (*woodver == BSP_HARDWARE_VERSION_BOLLYWOOD_PROD_FOR_WII) {
        //TODO: lots more stuff, checks on the Latte hw asic ver
    }

    wood_hardware_ver = *woodver;
    return 0;
}

/*  Something to do with getting the Wood version?
*   .text:e600b5b4
*/
int InitASICVersion(unsigned int* woodver) {
    int ret;

    *woodver = wood_hardware_ver;
    if (wood_hardware_ver) return 0;

    ret = GetWoodHardwareVersion(woodver, false); //.text:E600B41C - see below
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
    Decodes LT_ASICREV_ACR; placing the result in woodver.
    Doesn't seem to be an ASICREV_ACR revision for HOLLYWOOD_PROD_FOR_WII or
    HOLLYWOOD_CORTADO_ESPRESSO, this is determined from other factors */
int GetWoodHardwareVersion(unsigned int* woodver, bool doExtraHardwareVersions) {
    int ret;

    switch (*LT_ASICREV_ACR & 0xFF /*VERLO: revision*/) {
        case 0x00: {
            *woodver = BSP_HARDWARE_VERSION_HOLLYWOOD_ENG_SAMPLE_1;
            return 0;
        }
        case 0x10: {
            *woodver = BSP_HARDWARE_VERSION_HOLLYWOOD_ENG_SAMPLE_2;
            return 0;
        }
        case 0x11: {
            *woodver = BSP_HARDWARE_VERSION_CORTADO;
            if (!doExtraHardwareVersions) return 0;

            unsigned int isCortado;
            ret = CheckCortado(&isCortado); //.text:E6005818
            if (ret) return 0;

            if (!isCortado) {
                *woodver = BSP_HARDWARE_VERSION_HOLLYWOOD_PROD_FOR_WII;
                return 0;
            }

            sub_E6007C5C(&word_E6047988);
            *woodver = BSP_HARDWARE_VERSION_CORTADO;
            if (word_E6047988 == 0x7001) {
                *woodver = BSP_HARDWARE_VERSION_CORTADO_ESPRESSO;
            }
            return 0;
        }
        case 0x20: {
            *woodver = BSP_HARDWARE_VERSION_BOLLYWOOD;
            return 0;
        }
        case 0x21: {
            *woodver = BSP_HARDWARE_VERSION_BOLLYWOOD_PROD_FOR_WII;
            return 0;
        }
        default: {
            *woodver = BSP_HARDWARE_VERSION_UNKNOWN;
            return 0x800;
        }
    }
}

//.text:E600B328
int GetLatteHardwareVersion(unsigned int* woodver) {
    int ret;

    uint32_t asicrev_ccr = *LT_ASICREV_CCR;
    if (asicrev_ccr & 0xFFFF0000 != 0xCAFE0000) {
        return 0x800;
    }

    *woodver &= 0x000FFFFF;

    switch (asicrev_ccr & 0xFF) /* VERLO */ {
        case 0x10:
            *woodver |= 0x21100000; //Latte A11
            return 0;
        case 0x18:
            *woodver |= 0x21200000; //Latte A12
            return 0;
        case 0x21:
            *woodver |= 0x22100000; //Latte A2x
            return 0;
        case 0x30:
            *woodver |= 0x23100000; //Latte A3x
            return 0;
        case 0x40:
            *woodver |= 0x24100000; //Latte A4x
            return 0;
        case 0x50:
            *woodver |= 0x25100000; //Latte A5x
            return 0;
        default:
            *woodver |= 0x26100000; //Latte B1x
            return 0;
    }
}
