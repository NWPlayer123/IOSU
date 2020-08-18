#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "ipc.h"

typedef int32_t IOSError;
enum IOSError {
    IOS_ERROR_OK = 0x00000000,
    IOS_ERROR_ACCESS = 0xFFFFFFFF,
    IOS_ERROR_EXISTS = 0xFFFFFFFE,
    IOS_ERROR_INTR = 0xFFFFFFFD,
    IOS_ERROR_INVALID = 0xFFFFFFFC,
    IOS_ERROR_MAX = 0xFFFFFFFB,
    IOS_ERROR_NOEXISTS = 0xFFFFFFFA,
    IOS_ERROR_QEMPTY = 0xFFFFFFF9,
    IOS_ERROR_QFULL = 0xFFFFFFF8,
    IOS_ERROR_UNKNOWN = 0xFFFFFFF7,
    IOS_ERROR_NOTREADY = 0xFFFFFFF6,
    IOS_ERROR_ECC = 0xFFFFFFF5,
    IOS_ERROR_ECC_CRIT = 0xFFFFFFF4,
    IOS_ERROR_BADBLOCK = 0xFFFFFFF3,
    IOS_ERROR_INVALID_OBJTYPE = 0xFFFFFFF2,
    IOS_ERROR_INVALID_RNG = 0xFFFFFFF1,
    IOS_ERROR_INVALID_FLAG = 0xFFFFFFF0,
    IOS_ERROR_INVALID_FORMAT = 0xFFFFFFEF,
    IOS_ERROR_INVALID_VERSION = 0xFFFFFFEE,
    IOS_ERROR_INVALID_SIGNER = 0xFFFFFFED,
    IOS_ERROR_FAIL_CHECKVALUE = 0xFFFFFFEC,
    IOS_ERROR_FAIL_INTERNAL = 0xFFFFFFEB,
    IOS_ERROR_FAIL_ALLOC = 0xFFFFFFEA,
    IOS_ERROR_INVALID_SIZE = 0xFFFFFFE9,
    IOS_ERROR_NO_LINK = 0xFFFFFFE8,
    IOS_ERROR_AN_FAILED = 0xFFFFFFE7,
    IOS_ERROR_MAX_SEM_COUNT = 0xFFFFFFE6,
    IOS_ERROR_SEM_UNAVAILABLE = 0xFFFFFFE5,
    IOS_ERROR_INVALID_HANDLE = 0xFFFFFFE4,
    IOS_ERROR_INVALID_ARG = 0xFFFFFFE3,
    IOS_ERROR_NO_RESOURCE = 0xFFFFFFE2,
    IOS_ERROR_BUSY = 0xFFFFFFE1,
    IOS_ERROR_TIMEOUT = 0xFFFFFFE0,
    IOS_ERROR_ALIGNMENT = 0xFFFFFFDF,
    IOS_ERROR_BSP = 0xFFFFFFDE,
    IOS_ERROR_DATA_PENDING = 0xFFFFFFDD,
    IOS_ERROR_EXPIRED = 0xFFFFFFDC,
    IOS_ERROR_NO_R_ACCESS = 0xFFFFFFDB,
    IOS_ERROR_NO_W_ACCESS = 0xFFFFFFDA,
    IOS_ERROR_NO_RW_ACCESS = 0xFFFFFFD9,
    IOS_ERROR_CLIENT_TXN_LIMIT = 0xFFFFFFD8,
    IOS_ERROR_STALE_HANDLE = 0xFFFFFFD7,
    IOS_ERROR_UNKNOWN_VALUE = 0xFFFFFFD6,
};

typedef int32_t IOSProcessId;
enum IOSProcessId {
    ACP=8,
    AUXIL=10,
    BSP=2,
    COS02=16,
    COS03=17,
    COSERROR=20,
    COSHBM=19,
    COSKERNEL=14,
    COSMASTER=21,
    COSOVERLAY=18,
    COSROOT=15,
    CRYPTO=3,
    FPD=12,
    FS=5,
    Invalid=0,
    KERNEL=0,
    MCP=1,
    Max=22,
    NET=7,
    NIM=11,
    NSEC=9,
    PAD=6,
    TEST=13,
    USB=4
};

IOSError IOS_GetCurrentProcessId(void);
IOSError IOS_GetProcessName(IOSProcessId process, char * buffer);

typedef int32_t IOSMessageQueue;
IOSMessageQueue IOS_CreateMessageQueue(void* mem, int num_messages);
IOSError IOS_ReceiveMessage(IOSMessageQueue queue, IOSMessage** msg, int flags);
IOSError IOS_ResourceReply(IOSMessage* msg, int32_t reply);
IOSError IOS_RegisterResourceManager(const char* device, IOSMessageQueue queue);
IOSError IOS_DeviceAssosciate(const char* device, int id);

typedef int32_t IOSCrossProcessHeap;
IOSCrossProcessHeap IOS_CreateCrossProcessHeap(size_t size);

typedef int32_t IOSSemaphore;
IOSError IOS_CreateSemaphore(int32_t maxCount, int32_t initialCount);
IOSError IOS_WaitSemaphore(IOSSemaphore id, bool tryWait);
IOSError IOS_SignalSemaphore(IOSSemaphore id);
IOSError IOS_DestroySemaphore(IOSSemaphore id);

void IOS_SetBSPReady();
