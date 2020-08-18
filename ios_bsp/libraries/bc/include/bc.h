#pragma once

#include <gctypes.h>

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
