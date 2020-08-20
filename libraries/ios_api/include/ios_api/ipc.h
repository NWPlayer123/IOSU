#pragma once

#include <stdint.h>
#include "ios.h"

typedef uint32_t IOSMessageCommand;
enum IOSMessageCommand {
    IOS_COMMAND_INVALID = 0,
    IOS_OPEN            = 1,
    IOS_CLOSE           = 2,
    IOS_READ            = 3,
    IOS_WRITE           = 4,
    IOS_SEEK            = 5,
    IOS_IOCTL           = 6,
    IOS_IOCTLV          = 7,
    IOS_REPLY           = 8,
    IOS_IPC_MSG0        = 9,
    IOS_IPC_MSG1        = 10,
    IOS_IPC_MSG2        = 11,
    IOS_SUSPEND         = 12,
    IOS_RESUME          = 13,
    IOS_SVCMSG          = 14,
};

typedef uint32_t IOSMessageClientCPU;
enum IOSMessageClientCPU {
    IOS_ARM  = 0,
    IOS_PPC0 = 1,
    IOS_PPC1 = 2,
    IOS_PPC2 = 3,
};

typedef uint32_t IOSOpenMode;
enum IOSOpenMode {
    IOS_OPEN_MODE_NONE = 0,
    IOS_OPEN_MODE_READ = 1,
    IOS_OPEN_MODE_WRITE = 2,
};

typedef struct IOSMessage { /* IOS IPC message */
    IOSMessageCommand command;
    uint32_t clientReply;
    int32_t clientFD;
    uint32_t flags; /* Invalid if command=IOS_OPEN */
    IOSMessageClientCPU clientCPU;
    uint32_t clientPID;
    uint64_t clientTID;
    uint32_t serverHandle;
/*  If cmd = ...
    IOS_OPEN: name,
    IOS_READ: outPtr,
    IOS_WRITE: inPtr,
    IOS_SEEK: where,
    IOS_IOCTL: cmd,
    IOS_IOCTLV: cmd */
    uint32_t arg0;
/*  If cmd = ...
    IOS_OPEN: name_size,
    IOS_READ: outLen,
    IOS_WRITE: inLen,
    IOS_SEEK: whence,
    IOS_IOCTL: inPtr,
    IOS_IOCTLV: readCount */
    uint32_t arg1;
/*  If cmd = ...
    IOS_OPEN: mode, (IOSOpenMode)
    IOS_IOCTL: inLen,
    IOS_IOCTLV: writeCount */
    uint32_t arg2;
/*  If cmd = ...
    IOS_OPEN: permissions_bitmask, (upper)
    IOS_IOCTL: outPtr,
    IOS_IOCTLV: vector */
    uint32_t arg3;
/*  If cmd = ...
    IOS_OPEN: permissions_bitmask, (lower)
    IOS_IOCTL: outLen */
    uint32_t arg4;
    IOSMessageCommand prevCommand;
    uint32_t prevClientFD;
    void * ppcVirt0;
    void * ppcVirt1;
} IOSMessage;
