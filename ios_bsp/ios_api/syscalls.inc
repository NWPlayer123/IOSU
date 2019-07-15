/*  Call _syscall(name, number). keep in order of ID#!
    5.5.x retail. TODO: other versions?
    Make sure to also update _syscalls.h */

_syscall(IOS_CreateMessageQueue, 0xC)
_syscall(IOS_ReceiveMessage, 0x10)
_syscall(IOS_CreateCrossProcessHeap, 0x25)
_syscall(IOS_RegisterResourceManager, 0x2C)
_syscall(IOS_DeviceAssosciate, 0x2D)
_syscall(IOS_ResourceReply, 0x49)
