#pragma once
#include "bsp_hwrevs.h"

#include <stdint.h>
#include <stddef.h>

typedef struct BSP_IOCTL_METHOD {
    char entityName[32];
    uint32_t instance; //+0x20
    char attributeName[32]; //+0x24
    size_t size; //+0x44
    char inData[]; //+0x48...
} BSP_IOCTL_METHOD;

enum BSP_IOCTL {
    BSP_IOCTL_GET_ENTITY_VERSION = 1,
    BSP_IOCTL_GET_HARDWARE_VERSION = 2,
    BSP_IOCTL_GET_CONSOLE_MASK = 3,
    BSP_IOCTL_METHOD_QUERY = 4,
    BSP_IOCTL_METHOD_READ = 5,
    BSP_IOCTL_METHOD_WRITE = 6,
    BSP_IOCTL_METHOD_INIT = 7,
    BSP_IOCTL_METHOD_SHUTDOWN = 8,
    BSP_IOCTL_GET_CONSOLE_TYPE = 9,
};

typedef struct BSP_SYSTEM_CLOCK_INFO {
    uint32_t systemClockFrequency;
    uint32_t timerFrequency;
} BSP_SYSTEM_CLOCK_INFO;

typedef struct BSP_PPC_SUMMARY {
    uint32_t numberOfCores;
    uint32_t activeCoreBitmap;
    BSP_SYSTEM_CLOCK_INFO clock60x;
} BSP_PPC_SUMMARY;
//sizeof(BSP_PPC_SUMMARY) = 0x10

typedef struct BSP_PPC_CORE_PROPERTIES {
    uint32_t l2Size;
    uint32_t l2LineSize;
    uint32_t l2SectorSize;
    uint32_t l2FetchSize;
    uint32_t l2SetAssociativity;
    uint8_t coreActive;
} BSP_PPC_CORE_PROPERTIES;
//sizeof(BSP_PPC_CORE_PROPERTIES) = 0x15

typedef struct BSP_PPC_PVR {
    uint16_t version;
    uint16_t revision;
} BSP_PPC_PVR;
//sizeof(BSP_PPC_PVR) = 0x4

typedef struct BSP_HARDWARE_INFO {
    BSP_SYSTEM_CLOCK_INFO clock;
    BSP_HARDWARE_VERSION hwver;
} BSP_HARDWARE_INFO;
//sizeof(BSP_HARDWARE_INFO) = 0xC

extern BSP_HARDWARE_INFO bspHwInfo;
