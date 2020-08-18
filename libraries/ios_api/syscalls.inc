/*  Call _syscall(name, number). keep in order of ID#!
    5.5.x retail. TODO: other versions?
    Make sure to also update _syscalls.h */

_syscall(IOS_GetCurrentProcessId, 0x5)
_syscall(IOS_GetProcessName, 0x6)
_syscall(IOS_CreateMessageQueue, 0xC)
_syscall(IOS_ReceiveMessage, 0x10)
_syscall(IOS_CreateCrossProcessHeap, 0x25)
_syscall(IOS_RegisterResourceManager, 0x2C)
_syscall(IOS_DeviceAssosciate, 0x2D)
_syscall(IOS_ResourceReply, 0x49)
_syscall(IOS_VirtToPhys, 0x56)
_syscall(IOS_CreateSemaphore, 0x57)
_syscall(IOS_WaitSemaphore, 0x58)
_syscall(IOS_SignalSemaphore, 0x59)
_syscall(IOS_DestroySemaphore, 0x5A)
_syscall(IOS_SetBSPReady, 0x5C)
