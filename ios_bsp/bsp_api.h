#pragma once
#include "bsp_api_enum.h"

typedef struct BSP_ATTRIBUTE_TAG {
    char * pName;
    enum BSP_ATTRIBUTE_OPTIONS options;
    enum BSP_ATTRIBUTE_PERMISSIONS permissions;
    u32 data;
    ulong dataSize;
    BSP_RVAL (* readMethod)(u32, struct BSP_ATTRIBUTE_TAG *, void *);
    BSP_RVAL (* queryMethod)(u32, struct BSP_ATTRIBUTE_TAG *, void *);
    BSP_RVAL (* writeMethod)(u32, struct BSP_ATTRIBUTE_TAG *, void *);
    BSP_RVAL (* initMethod)(u32, struct BSP_ATTRIBUTE_TAG *, void *, ulong);
    BSP_RVAL (* shutdownMethod)(u32, struct BSP_ATTRIBUTE_TAG *);
    u32 attribute_specific;
} BSP_ATTRIBUTE;

typedef u32 BSP_ENTITY_VERSION;

typedef struct BSP_ENTITY {
    char * pName;
    BSP_ENTITY_VERSION version;
    u32 instanceCount;
    enum BSP_ENTITY_OPTIONS options;
    BSP_ATTRIBUTE * pAttributes[];
} BSP_ENTITY;
