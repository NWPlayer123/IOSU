#include "bsp_entity.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

int bspEntityCount; //.bss:e6047204
BSP_ENTITY* bspEntityList[32]; //.bss:e6047208

BSP_RVAL bspFindEntity(const char* entityName, BSP_ENTITY **ppE) {
    *ppE = NULL;
    if (bspEntityCount == 0) {
        return BSP_RVAL_UNKNOWN_ENTITY;
    }

    //if (bspEntityCount < 0) return BSP_RVAL_OK;

    for (int i = 0; i < bspEntityCount; i++) {
        int ret = strncmp(entityName, bspEntityList[i]->pName, 32);
        if (ret == 0) {
            *ppE = bspEntityList[i];
            return BSP_RVAL_OK;
        }
    /*  I'm not entirely sure what the logic behind this one is */
        if (ret < 0) {
            return BSP_RVAL_UNKNOWN_ENTITY;
        }
    }

    return BSP_RVAL_OK;
}

BSP_RVAL bspFindAttribute(const char* entityName, u32 instance,
    const char* attributeName, BSP_ENTITY **ppE, BSP_ATTRIBUTE **ppA,
    BSP_CLIENT_CREDENTIALS clientCredentials, u32 attributeCommand) {

    BSP_RVAL ret;
    *ppA = NULL;
    *ppE = NULL;

/*  I had to use a fair amount of artistic license to get this readable.
    I removed a lot of checks that seem to always pass when *ppA is
    NULL. I dunno if the compiler just wasn't feeling it that day or if I'm
    missing something (I suspect the former). */
    ret = bspFindEntity(entityName, ppE);
    if (ret == BSP_RVAL_OK) {
        BSP_ENTITY* entity = *ppE;
        if (instance >= entity->instanceCount) {
            return BSP_RVAL_INVALID_INSTANCE;
        }
        if (attributeName[0] == '\0') {
            return BSP_RVAL_OK;
        }

        BSP_ATTRIBUTE* attrib;
        if (entity->options & BSP_EO_FLAT_INSTANCES) {
            attrib = entity->pAttributes[0];
        } else {
            attrib = entity->pAttributes[instance];
        }

        for (;;) {
            if (attrib->pName == NULL) {
                return BSP_RVAL_UNKNOWN_ATTRIBUTE;
            }

            if (strncmp(attributeName, attrib->pName, 32) == 0) {
                *ppA = attrib;
                break;
            }

            attrib++;
        }
    }

    if (clientCredentials & BSP_PERMISSIONS_INTERNAL) {
    /*  Skip permission check */
        return ret;
    }

    if (*ppA == NULL) return ret;
    enum BSP_ATTRIBUTE_PERMISSIONS permissions = (*ppA)->permissions;

    if (permissions & BSP_PERMISSIONS_INTERNAL) {
        permissions = BSP_PERMISSIONS_INTERNAL;
    }

    bool ok = false;
    switch (attributeCommand) {
        case BSP_IOCTL_METHOD_QUERY:
        case BSP_IOCTL_METHOD_READ: {
        /*  Mask client credentials for relevant bits, then check */
            if (clientCredentials & (
                BSP_PERMISSIONS_INTERNAL |
                BSP_PERMISSIONS_PPC_SUPV_READ |
                BSP_PERMISSIONS_PPC_USER_READ |
                BSP_PERMISSIONS_IOS_SUPV_READ |
                BSP_PERMISSIONS_IOS_USER_READ
            ) & permissions) {
                ok = true;
            }
            break;
        }
        case BSP_IOCTL_METHOD_WRITE: {
        /*  Mask client credentials for relevant bits, then check */
            if (clientCredentials & (
                BSP_PERMISSIONS_INTERNAL |
                BSP_PERMISSIONS_PPC_SUPV_WRITE |
                BSP_PERMISSIONS_PPC_USER_WRITE |
                BSP_PERMISSIONS_IOS_SUPV_WRITE |
                BSP_PERMISSIONS_IOS_USER_WRITE
            ) & permissions) {
                ok = true;
            }
            break;
        }
        case BSP_IOCTL_METHOD_INIT: {
        /*  Mask client credentials for relevant bits, then check */
            if (clientCredentials & (
                BSP_PERMISSIONS_INTERNAL |
                BSP_PERMISSIONS_PPC_SUPV_INIT |
                BSP_PERMISSIONS_PPC_USER_INIT |
                BSP_PERMISSIONS_IOS_SUPV_INIT |
                BSP_PERMISSIONS_IOS_USER_INIT
            ) & permissions) {
                ok = true;
            }
            break;
        }
        case BSP_IOCTL_METHOD_SHUTDOWN: {
        /*  Mask client credentials for relevant bits, then check */
            if (clientCredentials & (
                BSP_PERMISSIONS_INTERNAL |
                BSP_PERMISSIONS_PPC_SUPV_SHUTDOWN |
                BSP_PERMISSIONS_PPC_USER_SHUTDOWN |
                BSP_PERMISSIONS_IOS_SUPV_SHUTDOWN |
                BSP_PERMISSIONS_IOS_USER_SHUTDOWN
            ) & permissions) {
                ok = true;
            }
            break;
        }
        default: {
            return ret | BSP_RVAL_REQUEST_DENIED;
        }
    }

    if (!ok) return ret | BSP_RVAL_REQUEST_DENIED;
    return ret;
}

BSP_RVAL bspMethodRead(const char* entityName, u32 instance,
    const char* attributeName, size_t size, void* pReadData,
    BSP_CLIENT_CREDENTIALS clientCredentials) {

    BSP_RVAL ret;
    BSP_ENTITY* pE;
    BSP_ATTRIBUTE* pA;

    ret = bspFindAttribute(entityName, instance, attributeName, &pE, &pA,
        clientCredentials, BSP_IOCTL_METHOD_READ);
    if (ret != BSP_RVAL_OK) return ret;

    if (size != pA->dataSize) {
        return BSP_RVAL_SPECIFIED_SIZE_INVALID;
    }
    if (pA->readMethod == NULL) {
        return BSP_RVAL_UNSUPPORTED_METHOD;
    }

    return pA->readMethod(instance, pA, pReadData);
}
BSP_RVAL bspReadInternal(const char* entityName, u32 instance,
    const char* attributeName, size_t size, void* pReadData) {

    return bspMethodRead(entityName, instance, attributeName, size, pReadData,
        BSP_PERMISSIONS_INTERNAL);
}

BSP_RVAL bspMethodQuery(const char* entityName, u32 instance,
    const char* attributeName, size_t size, void* pQueryData,
    BSP_CLIENT_CREDENTIALS clientCredentials) {

    BSP_RVAL ret;
    BSP_ENTITY* pE;
    BSP_ATTRIBUTE* pA;

    ret = bspFindAttribute(entityName, instance, attributeName, &pE, &pA,
        clientCredentials, BSP_IOCTL_METHOD_QUERY);
    if (ret != BSP_RVAL_OK) return ret;

    if (size != pA->dataSize) {
        return BSP_RVAL_SPECIFIED_SIZE_INVALID;
    }

/*  Case 1: attribute has a query method */
    if (pA->queryMethod) {
        return pA->queryMethod(instance, pA, pQueryData);
    }

/*  Case 2: Direct data query (read from pA->data) */
    if (pA->options & BSP_AO_QUERY_DATA_DIRECT) {
        if (size > 4) {
            return BSP_RVAL_CFG_CORRUPTED;
        }

    /*  e.g. for size = 1, read 1 byte from data + 3; for size 2, read from
        data + 2, etc. */
        void* data = (void*)(&pA->data) + 4 - size;
        memcpy(pQueryData, data, size);
        return BSP_RVAL_OK;
    }

/*  Case 2: Indirect data query (read from *(pA->data)) */
    if (pA->options & BSP_AO_QUERY_DATA_INDIRECT) {
        memcpy(pQueryData, (void*)pA->data, size);
        return BSP_RVAL_OK;
    }

    return BSP_RVAL_QUERY_UNAVAILABLE;
}
BSP_RVAL bspQueryInternal(const char* entityName, u32 instance,
    const char* attributeName, size_t size, void* pQueryData) {

    return bspMethodQuery(entityName, instance, attributeName, size, pQueryData,
        BSP_PERMISSIONS_INTERNAL);
}

BSP_RVAL bspMethodWrite(const char* entityName, u32 instance,
    const char* attributeName, size_t size, void* pWrittenData,
    BSP_CLIENT_CREDENTIALS clientCredentials) {

    BSP_RVAL ret;
    BSP_ENTITY* pE;
    BSP_ATTRIBUTE* pA;

    ret = bspFindAttribute(entityName, instance, attributeName, &pE, &pA,
        clientCredentials, BSP_IOCTL_METHOD_WRITE);
    if (ret != BSP_RVAL_OK) return ret;

    if (size != pA->dataSize) {
        return BSP_RVAL_SPECIFIED_SIZE_INVALID;
    }
    if (pA->writeMethod == NULL) {
        return BSP_RVAL_UNSUPPORTED_METHOD;
    }

    return pA->writeMethod(instance, pA, pWrittenData);
}
BSP_RVAL bspWriteInternal(const char* entityName, u32 instance,
    const char* attributeName, size_t size, void* pWrittenData) {

    return bspMethodWrite(entityName, instance, attributeName, size, pWrittenData,
        BSP_PERMISSIONS_INTERNAL);
}

BSP_RVAL bspMethodInit(const char* entityName, u32 instance,
    const char* attributeName, size_t size, void* pInitData,
    BSP_CLIENT_CREDENTIALS clientCredentials) {

    BSP_RVAL ret;
    BSP_ENTITY* pE;
    BSP_ATTRIBUTE* pA;

    ret = bspFindAttribute(entityName, instance, attributeName, &pE, &pA,
        clientCredentials, BSP_IOCTL_METHOD_INIT);
    if (ret != BSP_RVAL_OK) return ret;

/*  Normal case: given an attribute to init */
    if (attributeName[0] != '\0') {
        if (!pA->initMethod) {
            return BSP_RVAL_UNSUPPORTED_METHOD;
        }
        return pA->initMethod(instance, pA, pInitData, size);
    }

/*  Special case: BSP_AO_INIT_GLOBAL */
    if (pE->options & BSP_EO_FLAT_INSTANCES) {
        pA = pE->pAttributes[0];
    } else {
        pA = pE->pAttributes[instance];
    }

    if (pA->pName == NULL) {
        return BSP_RVAL_OK;
    }

    while (pA->pName != NULL) {
        if (pA->initMethod == NULL) continue;
        if (!(pA->options & BSP_AO_INIT_GLOBAL)) continue;

        ret |= pA->initMethod(instance, pA, NULL, 0);
        pA++;
    }
    return ret;
}
BSP_RVAL bspInitInternal(const char* entityName, u32 instance,
    const char* attributeName, size_t size, void* pInitData) {

    return bspMethodInit(entityName, instance, attributeName, size, pInitData,
        BSP_PERMISSIONS_INTERNAL);
}

BSP_RVAL bspMethodShutdown(const char* entityName, u32 instance,
    const char* attributeName, BSP_CLIENT_CREDENTIALS clientCredentials) {

    BSP_RVAL ret;
    BSP_ENTITY* pE;
    BSP_ATTRIBUTE* pA;

    ret = bspFindAttribute(entityName, instance, attributeName, &pE, &pA,
        clientCredentials, BSP_IOCTL_METHOD_SHUTDOWN);
    if (ret != BSP_RVAL_OK) return ret;

    if (pA->shutdownMethod == NULL) {
        return BSP_RVAL_UNSUPPORTED_METHOD;
    }

    return pA->shutdownMethod(instance, pA);
}
BSP_RVAL bspShutdownInternal(const char* entityName, u32 instance,
    const char* attributeName) {

    return bspMethodShutdown(entityName, instance, attributeName, BSP_PERMISSIONS_INTERNAL);
}

BSP_RVAL bspGetEntityVersion(const char* entityName, BSP_ENTITY_VERSION* version) {
    BSP_RVAL ret;
    BSP_ENTITY* pE;

    ret = bspFindEntity(entityName, &pE);
    if (ret != BSP_RVAL_OK) return ret;

    memcpy(version, &pE->version, sizeof(pE->version));
    return BSP_RVAL_OK;
}
