#pragma once
#include "bsp_hwrevs.h"

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
}
