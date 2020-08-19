# needs -x assembler-with-cpp, woo
.text

.macro _asmsyscall name, num
.arm
.global \name
\name:
    .word 0xE7F000F0 | \num << 8
.endm

.thumb
.global __sys_write0
__sys_write0:
    mov r2, lr
    adds r1, r0, $0
    movs r0, $4
    svc 0xab
    bx r2

#define _syscall(name, num) _asmsyscall name, num
#include "syscalls.inc"
