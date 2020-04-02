#pragma once

// https://wiiubrew.org/wiki/Hardware/Latte_Registers#HW_IOPWRCTRL
#define HW_IOPWRCTRL ((uint32_t*)0x0d8001dc)
    #define HW_IOPWRCTRL_BIT(dev, val) (val << (dev * 2))
    #define HW_IOPWRCTRL_MASK 2
    #define HW_IOPWRCTRL_DI(val)    HW_IOPWRCTRL_BIT(3, val)
    #define HW_IOPWRCTRL_VI(val)    HW_IOPWRCTRL_BIT(4, val)
    #define HW_IOPWRCTRL_AI(val)    HW_IOPWRCTRL_BIT(6, val)
    #define HW_IOPWRCTRL_SI(val)    HW_IOPWRCTRL_BIT(9, val)
    #define HW_IOPWRCTRL_SDIO(val)  HW_IOPWRCTRL_BIT(10, val)
    #define HW_IOPWRCTRL_FLA(val)   HW_IOPWRCTRL_BIT(11, val)
    #define HW_IOPWRCTRL_GPIO(val)  HW_IOPWRCTRL_BIT(12, val)
    #define HW_IOPWRCTRL_GPIOB(val) HW_IOPWRCTRL_BIT(13, val)

// https://wiiubrew.org/wiki/Hardware/Latte_Registers#HW_IOSTRCTRL0
#define HW_IOSTRCTRL0 ((uint32_t*)0x0d8001e0)
    #define HW_IOSTRCTRL0_BIT(dev, val) (val << (dev * 3))
    #define HW_IOSTRCTRL0_MASK 3
    #define HW_IOSTRCTRL0_AI(val) HW_IOSTRCTRL0_BIT(0, val)
    #define HW_IOSTRCTRL0_SI(val) HW_IOSTRCTRL0_BIT(2, val)
    #define HW_IOSTRCTRL0_DI(val) HW_IOSTRCTRL0_BIT(6, val)
    #define HW_IOSTRCTRL0_VI(val) HW_IOSTRCTRL0_BIT(7, val)

// https://wiiubrew.org/wiki/Hardware/Latte_Registers#HW_IOSTRCTRL0
#define HW_IOSTRCTRL1 ((uint32_t*)0x0d8001e4)
    #define HW_IOSTRCTRL1_BIT(dev, val) (val << (dev * 3))
    #define HW_IOSTRCTRL1_MASK 3
    #define HW_IOSTRCTRL1_SDIO(val) HW_IOSTRCTRL1_BIT(7, val)

#define LT_IOP2X ((uint32_t*)0x0d8005bc)
    #define LT_IOP2X_ENABLE 0x04
