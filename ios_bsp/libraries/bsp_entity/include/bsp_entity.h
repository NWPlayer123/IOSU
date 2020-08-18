#pragma once
#include <gctypes.h>

#include "bsp_entity_enum.h"

struct BSP_ATTRIBUTE;

typedef struct BSP_ATTRIBUTE {
    char * pName;
    enum BSP_ATTRIBUTE_OPTIONS options;
    enum BSP_ATTRIBUTE_PERMISSIONS permissions;
    u32 data;
    ulong dataSize;
    BSP_RVAL (* readMethod)(u32, struct BSP_ATTRIBUTE *, void *);
    BSP_RVAL (* queryMethod)(u32, struct BSP_ATTRIBUTE *, void *);
    BSP_RVAL (* writeMethod)(u32, struct BSP_ATTRIBUTE *, void *);
    BSP_RVAL (* initMethod)(u32, struct BSP_ATTRIBUTE *, void *, ulong);
    BSP_RVAL (* shutdownMethod)(u32, struct BSP_ATTRIBUTE *);
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

BSP_RVAL bspMethodRead(const char* entityName, u32 instance, const char* attributeName, size_t size, void* pReadData, BSP_CLIENT_CREDENTIALS clientCredentials);
BSP_RVAL bspReadInternal(const char* entityName, u32 instance, const char* attributeName, size_t size, void* pReadData);

BSP_RVAL bspMethodQuery(const char* entityName, u32 instance, const char* attributeName, size_t size, void* pQueryData, BSP_CLIENT_CREDENTIALS clientCredentials);
BSP_RVAL bspQueryInternal(const char* entityName, u32 instance, const char* attributeName, size_t size, void* pQueryData);

BSP_RVAL bspMethodWrite(const char* entityName, u32 instance, const char* attributeName, size_t size, void* pWrittenData, BSP_CLIENT_CREDENTIALS clientCredentials);
BSP_RVAL bspWriteInternal(const char* entityName, u32 instance, const char* attributeName, size_t size, void* pWrittenData);

BSP_RVAL bspMethodInit(const char* entityName, u32 instance, const char* attributeName, size_t size, void* pInitData, BSP_CLIENT_CREDENTIALS clientCredentials);
BSP_RVAL bspInitInternal(const char* entityName, u32 instance, const char* attributeName, size_t size, void* pInitData);

BSP_RVAL bspMethodShutdown(const char* entityName, u32 instance, const char* attributeName, BSP_CLIENT_CREDENTIALS clientCredentials);
BSP_RVAL bspShutdownInternal(const char* entityName, u32 instance, const char* attributeName);
