#pragma once

#include <gctypes.h>
#include <ios_api/ios.h>

typedef struct BC_CONFIG {
    u32 crc;
    u16 size;
    u16 version;
    u16 author;
    u16 boardType;
    u16 boardRevision;
    u16 bootSource;
    u16 ddr3Size;
    u16 ddr3Speed;
    u16 ppcClockMultiplier;
    u16 iopClockMultiplier;
    u16 video1080p;
    u16 ddr3Vendor;
    u16 movPassiveReset;
    u16 sysPllSpeed;
    u16 sataDevice;
    u16 consoleType;
    u32 devicePresence; /* v4+ only */
} BC_CONFIG;

typedef enum BC_CONSOLE_TYPE {
    BC_CONSOLE_TYPE_WUP          = 1,
    BC_CONSOLE_TYPE_CAT_R        = 2,
    BC_CONSOLE_TYPE_CAT_DEV      = 3,
    BC_CONSOLE_TYPE_EV           = 4,
    BC_CONSOLE_TYPE_CAT_I        = 5,
    BC_CONSOLE_TYPE_ORCHESTRA_X  = 6,
    BC_CONSOLE_TYPE_WUIH         = 7,
    BC_CONSOLE_TYPE_WUIH_DEV     = 8,
    BC_CONSOLE_TYPE_CAT_DEV_WUIH = 9,
} BC_CONSOLE_TYPE;

IOSError bcInit();
IOSError bcGet(BC_CONFIG* config);
