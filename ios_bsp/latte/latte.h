#pragma once

#define REG_BIT(bit, val, width) (val << (dev * width))

// https://wiiubrew.org/wiki/Hardware/Latte_Registers#HW_IOPWRCTRL
#define HW_IOPWRCTRL ((uint32_t*)0x0d8001dc)
    #define HW_IOPWRCTRL_MASK 2
    #define HW_IOPWRCTRL_DI(val)    REG_BIT(3, val, HW_IOPWRCTRL_MASK)
    #define HW_IOPWRCTRL_VI(val)    REG_BIT(4, val, HW_IOPWRCTRL_MASK)
    #define HW_IOPWRCTRL_AI(val)    REG_BIT(6, val, HW_IOPWRCTRL_MASK)
    #define HW_IOPWRCTRL_SI(val)    REG_BIT(9, val, HW_IOPWRCTRL_MASK)
    #define HW_IOPWRCTRL_SDIO(val)  REG_BIT(10, val, HW_IOPWRCTRL_MASK)
    #define HW_IOPWRCTRL_FLA(val)   REG_BIT(11, val, HW_IOPWRCTRL_MASK)
    #define HW_IOPWRCTRL_GPIO(val)  REG_BIT(12, val, HW_IOPWRCTRL_MASK)
    #define HW_IOPWRCTRL_GPIOB(val) REG_BIT(13, val, HW_IOPWRCTRL_MASK)

// https://wiiubrew.org/wiki/Hardware/Latte_Registers#HW_IOSTRCTRL0
#define HW_IOSTRCTRL0 ((uint32_t*)0x0d8001e0)
    #define HW_IOSTRCTRL0_MASK 3
    #define HW_IOSTRCTRL0_AI(val) REG_BIT(0, val, HW_IOSTRCTRL0_MASK)
    #define HW_IOSTRCTRL0_SI(val) REG_BIT(2, val, HW_IOSTRCTRL0_MASK)
    #define HW_IOSTRCTRL0_DI(val) REG_BIT(6, val, HW_IOSTRCTRL0_MASK)
    #define HW_IOSTRCTRL0_VI(val) REG_BIT(7, val, HW_IOSTRCTRL0_MASK)

// https://wiiubrew.org/wiki/Hardware/Latte_Registers#HW_IOSTRCTRL0
#define HW_IOSTRCTRL1 ((uint32_t*)0x0d8001e4)
    #define HW_IOSTRCTRL1_MASK 3
    #define HW_IOSTRCTRL1_SDIO(val) REG_BIT(7, val, HW_IOSTRCTRL1_MASK)

// https://wiibrew.org/wiki/Hardware/Hollywood_Registers#HW_AIPPROT
#define HW_AIP_PROT ((uint32_t*)0x0d800070)
    #define HW_AIP_PROT_ENAHBIOPI REG_BIT(0, 1, 1)

// https://wiibrew.org/wiki/Hardware/Hollywood_Registers#HW_RESETS
#define HW_RSTB ((uint32_t*)0x0d800194)
    #define HW_RSTB_CPU  REG_BIT(4, 1, 1)
    #define HW_SRSTB_CPU REG_BIT(5, 1, 1)
    #define HW_RSTB_PI   REG_BIT(9, 1, 1)

#define LT_IOP2X ((uint32_t*)0x0d8005bc)
    #define LT_IOP2X_ENABLE 0x04

#define LT_60XE_CFG ((uint32_t*)0x0d800640)
    #define LT_60XE_CFG_STREAMING REG_BIT(3, 1, 1)
    //entirely made up, could be not a latch
    #define LT_60XE_CFG_LATCH REG_BIT(12, 1, 1)

#define HW_EXI_BOOT ((uint32_t[16])0x0d806840)
