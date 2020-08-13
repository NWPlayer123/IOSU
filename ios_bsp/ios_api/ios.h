#pragma once

#include <stdint.h>
#include <stddef.h>
#include "ipc.h"

typedef int32_t IOSError ;
enum IOSError {
    IOS_ERROR_OK         = 0,
    IOS_ERROR_ACCESS     = 0xFFFFFFFF,
    IOS_ERROR_EXISTS     = 0xFFFFFFFE,
    IOS_ERROR_INVALID    = 0xFFFFFFFC,
    IOS_ERROR_NOEXISTS   = 0xFFFFFFFA,
};

typedef int32_t IOSMessageQueue;
IOSMessageQueue IOS_CreateMessageQueue(void* mem, int num_messages);
IOSError IOS_ReceiveMessage(IOSMessageQueue queue, IOSMessage** msg, int flags);
IOSError IOS_ResourceReply(IOSMessage* msg, int32_t reply);
IOSError IOS_RegisterResourceManager(const char* device, IOSMessageQueue queue);
IOSError IOS_DeviceAssosciate(const char* device, int id);

typedef int32_t IOSCrossProcessHeap;
IOSCrossProcessHeap IOS_CreateCrossProcessHeap(size_t size);

typedef IOSSemaphore int32_t;
IOSError IOS_CreateSemaphore(int32_t maxCount, int32_t initialCount);
IOSError IOS_WaitSemaphore(IOSSemaphore id, bool tryWait);
IOSError IOS_SignalSempahore(IOSSemaphore id);
IOSError IOS_DestroySempahore(IOSSemaphore id);

void IOS_SetBSPReady();
