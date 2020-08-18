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
