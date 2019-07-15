# needs -x assembler-with-cpp, woo
.text
.arm

.macro _asmsyscall name, num
.global \name
\name:
    .word 0xE7F000F0 | \num << 8
.endm

#define _syscall(name, num) _asmsyscall name, num
#include "syscalls.inc"
