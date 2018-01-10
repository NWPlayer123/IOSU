# Current Program Status Register (CPSR)
This is the register that's used to enable and disable interrupts, bits 6 and 7 (0xC0) for FIQ and IRQ interrupts.
https://en.wikipedia.org/wiki/ARM_architecture#Registers

```
IOS_KERNEL:0812E778 ; =============== S U B R O U T I N E =======================================
IOS_KERNEL:0812E778
IOS_KERNEL:0812E778 DisableInterrupts                             ; CODE XREF: sub_8120000+8↑p ...
IOS_KERNEL:0812E778                 MRS     R1, CPSR              # CPSR -> R1
IOS_KERNEL:0812E77C                 AND     R0, R1, #0xC0         # R0 = R1 & 0xC0 (add bits)
IOS_KERNEL:0812E780                 ORR     R1, R1, #0xC0         # R1 |= 0xC0
IOS_KERNEL:0812E784                 MSR     CPSR_c, R1            # R1 -> CPSR
IOS_KERNEL:0812E788                 BX      LR                    # Return R0 (to re-enable below ↓)
IOS_KERNEL:0812E788 ; End of function DisableInterrupts
IOS_KERNEL:0812E788
IOS_KERNEL:0812E78C ; =============== S U B R O U T I N E =======================================
IOS_KERNEL:0812E78C
IOS_KERNEL:0812E78C EnableInterrupts                              ; CODE XREF: sub_8120000+24↑p ...
IOS_KERNEL:0812E78C                 MRS     R1, CPSR              # CPSR -> R1
IOS_KERNEL:0812E790                 BIC     R1, R1, #0xC0         # R1 &= ~0xC0 (remove bits)
IOS_KERNEL:0812E794                 ORR     R1, R1, R0            # R1 |= R0 (re-enable from above ↑)
IOS_KERNEL:0812E798                 MSR     CPSR_c, R1            # R1 -> CPSR
IOS_KERNEL:0812E79C                 BX      LR                    # Return nothing
IOS_KERNEL:0812E79C ; End of function EnableInterrupts
IOS_KERNEL:0812E79C
```
