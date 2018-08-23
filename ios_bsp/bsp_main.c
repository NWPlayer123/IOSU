unsigned int devbsp_msgqueue[0x40]; //.bss:E6047000 - E60470FC inclusive
unsigned int devbsp_msgqueue_id; //.bss:E6047100
unsigned int devbsp_msgqueue_fdtable[0x40]; //.bss:E6047104 - E6047200 inclusive
int log_fd = -1; //.data:E6042000

/*  .text:E6000000
    Likely entry point. No obvious arguments/return values.
    Name taken from log output */
void main() {
/*  ==================================== initialisation bit */
    int ret, bsperr/*r9*/, ioserr/*r8*/;

    sub_E600E804();
    ret = IOS_CreateCrossProcessHeap(0x10000); //UND #0x250, .text:E600FA44
    if (ret < 0) {
    /*  Not 100% sure if this is a generic print or an error reporter */
        bsp_print("BSP: IOS_CreateCrossProcessHeap ERROR, rval = %d\n", ret); //.text:E600EC28
        return;
    }

    memset(devbsp_msgqueue_fdtable, 0, 0x100); //.text:E600EAB4

    devbsp_msgqueue_id = IOS_CreateMessageQueue(devbsp_msgqueue, 0x40); //UND #0xC0, .text:E600F97C
    if (devbsp_msgqueue_id < 0) {
        ioserr = devbsp_msgqueue_id;
        bsperr = 0;
        goto report_err;
    }

    ret = IOS_RegisterResourceManager("/dev/bsp", devbsp_msgqueue_id); //UND #0x2C0, .text:E600FA7C
    if (ret) {
        ioserr = ret;
        bsperr = 0;
        goto report_err;
    }

    ret = device_assosciate("/dev/bsp", 1); //UND #0x2D0, .text:E600FA84
    if (ret) {
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

    set_bsp_ready(); //UND #0x5C0, .text:E600FBFC

/*  ==================================== /dev/bsp processing bit */
    struct ipc_msg* msg/*sp+0x18,r7*/;
    unsigned int cmd, fd, result, perms_u, perms_l, i;
    unsigned int* fdtable_entry/*sp+0x14*/, inPtr, outPtr/*sp+0xC*/;

    for (;;) {
        ret = IOS_ReceiveMessage(devbsp_msgqueue_id, &msg, 0); //UND #0x100, .text:E600F99C
        if (ret) {
            ioserr = 0;
            bsperr = 0;
            break; //aka goto report_err;
        }
        cmd = __builtin_bswap32(msg->cmd);
        if (cmd == 2 /* close() */) {
            fd = __builtin_bswap32(msg->client_fd);
            if (fd > 0x1F || devbsp_msgqueue_fdtable[fd * 2] == 0) {
                result = 0xFFFFFFFA;
            } else {
                memset(&(devbsp_msgqueue_fdtable[fd * 2]), 0/*actually ioserr; yes, really*/, 8);
            }
        } else if (cmd == 6 /* ioctl() */) {
            fd = __builtin_bswap32(msg->client_fd);
            if (fd > 0x1F) {
                result = 0xFFFFFFFA;
                goto reply;
            }

        /*  in bytes, this is (fdtable + (fd << 3)) but I don't want to confuse
            by using pointer math */
            fdtable_entry = &(devbsp_msgqueue_fdtable[fd * 2]);
            if (!fdtable_entry[0]) {
                result = 0xFFFFFFFA;
                goto reply;
            }

        /*  ... these pointers never seem to be sanity-checked */
            cmd = __builtin_bswap32(msg->arg0); //arg0:cmd for ioctls
            inPtr = __builtin_bswap32(msg->arg1); //arg1:inPtr for ioctls
            outPtr = __builtin_bswap32(msg->arg3); //you get the idea

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
                    result = sub_E6000A90(inPtr, __builtin_bswap32(inPtr[8]/*+0x20*/), &(inPtr[9]/*+0x24*/), __builtin_bswap32(inPtr[0x11]/*+0x44*/) /*stack args, unsure of order*/ outPtr, fdtable_entry[1]);
                }
                case 0x5: {
                    result = sub_E6000A14(inPtr, __builtin_bswap32(inPtr[8]/*+0x20*/), &(inPtr[9]/*+0x24*/), __builtin_bswap32(inPtr[0x11]/*+0x44*/), /*stack args, unsure of order*/ outPtr, fdtable_entry[1]);
                }
                case 0x6: {
                    result = sub_E6000998(inPtr, __builtin_bswap32(inPtr[8]/*+0x20*/), &(inPtr[9]/*+0x24*/), __builtin_bswap32(inPtr[0x11]/*+0x44*/), &(inPtr[0x12]/*+0x48*/), fdtable_entry[1]);
                }
                case 0x7: {
                    result = sub_E6000890(inPtr, __builtin_bswap32(inPtr[8]/*+0x20*/), &(inPtr[9]/*+0x24*/), __builtin_bswap32(inPtr[0x11]/*+0x44*/), &(inPtr[0x12]/*+0x48*/), fdtable_entry[1]);
                }
                case 0x8: {
                    result = sub_E6000830(inPtr, __builtin_bswap32(inPtr[8]/*+0x20*/), &(inPtr[9]/*+0x24*/), fdtable_entry[1]);
                }
                case 0x9: {
                    if (__builtin_bswap32(inPtr[0x11]/*+0x44*/) != 4) {
                        result = 0x80;
                    } else {
                        result = sub_E600B270(outPtr);
                    }
                }
                default: {
                    result = 0xFFFFFFFC;
                }
            }
        } else if (cmd == 1 /* open() */) {
            perms_u = __builtin_bswap32(msg->arg3); //arg3:permissions_upper
            perms_l = __builtin_bswap32(msg->arg4); //arg4:permissions_lower

            fdtable_entry = devbsp_msgqueue_fdtable;
            for (i = 0; i != 0x20 /*yes, really*/; i++) {
                if (!fdtable_entry[0]) {
                    fdtable_entry[0] = 1;
                    fdtable_entry[1] = perms_l;
                    result = i;
                    goto reply;
                }
                fdtable_entry += 2; //pointer arithmetic: this adds 8 bytes
            }
            result = i - 0x3E;
        } else {
            result = 0xFFFFFFFC; //-4?
            goto reply;
        }

reply:
        IOS_ResourceReply(msg, result); //UND #0x490, .text:E600FB64
    }

report_err:
    sub_E600E264(log_fd, "main", "bsp_main.c", 0x97, /* args on stack */"main() is exiting, ioserr %d, bsp err 0x%x\n", ioserr, bsperr);
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
    
    error  = sub_E600B5B4(&wood_hardware_ver); //.text:E600B5B4 - see below
    error |= sub_E6006800(); //see below
    
    wood_hardware_ver = 0; //are you kidding me, Nintendo?
    
    error |= sub_E600B62C(&wood_hardware_ver); //see below
}

int sub_E6006800() {
    int ret;
    unsigned int woodver;
    
    ret = sub_E600B62C(&woodver); //.text:E600B62C - see below
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
*/
int sub_E600B5B4(unsigned int* woodver) {
    int ret;

    *woodver = wood_hardware_ver;
    if (!wood_hardware_ver) return 0;
    
    ret = GetWoodHardwareVersion(woodver, false); //.text:E600B41C - see below
    if (ret) return ret;
    
    if (*woodver == BSP_HARDWARE_VERSION_BOLLYWOOD_PROD_FOR_WII) {
        if (sub_E600B328() == 0) {
            *woodver = *woodver & 0xFFFF0000 | 0x11;
         /* aka:
            *arg1 = 0x20100011;
            This is not a documented hardware revision... */
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
    unsigned int isHollywoodProdForWii;
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
            
            stupidstackvar = 0;
            ret = CheckHollywoodProdForWii(&isHollywoodProdForWii); //.text:E6005818
            if (ret) return 0;
            
            if (stupidstackvar == 0) {
                *woodver = BSP_HARDWARE_VERSION_HOLLYWOOD_PROD_FOR_WII;
                return 0;
            }
            
            sub_E6007C5C(&word_E6047988);
            *woodver = BSP_HARDWARE_VERSION_CORTADO; //yes, really (???)
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