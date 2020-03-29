#include "bsp_hwrevs.h"

BSP_RVAL bspSDIO_IOStrength_init(u32 instance, BSP_ATTRIBUTE* pAttribute, void* pInitData, ulong size) {
    BSP_HARDWARE_VERSION hwver;
    BSP_RVAL err;
    uint32_t reg_hwstrctrl1, reg_ltstrctrl2, reg_unk05cc;

    err = bspMethodGetHardwareVersion(&hwver);
    if (err) return err;

    reg_hwstrctrl1 =  *HW_IOSTRCTRL1;
    reg_hwstrctrl1 &= ~HW_IOSTRCTRL1_SDIO(HW_IOSTRCTRL1_MASK);

/*  Pardon the spacing, this gets real hard to read otherwise */
    if (BSP_IS_HOLLYWOOD(hwver) || BSP_IS_HOLLYWOOD_ES1(hwver)) {
        reg_hwstrctrl1 |= HW_IOSTRCTRL1_SDIO(2);

    } else if (BSP_IS_BOLLYWOOD(hwver)) {
        reg_hwstrctrl1 |= HW_IOSTRCTRL1_SDIO(1);

    } else if (BSP_IS_LATTE(hwver)) {
        reg_ltstrctrl2 =  *LT_IOSTRCTRL2;
        reg_ltstrctrl2 &= ~(
            LT_IOSTRCTRL2_SDIOB(LT_IOSTRCTRL2_MASK) |
            LT_IOSTRCTRL2_SDIOC(LT_IOSTRCTRL2_MASK) |
            LT_IOSTRCTRL2_SDIOD(LT_IOSTRCTRL2_MASK)
        );

        reg_unk05cc = *(uint32_t*)0x0d8005cc;

        if (BSP_IS_EV_Y(hwver)) {
            reg_ltstrctrl2 |= LT_IOSTRCTRL2_SDIOB(4) |
                LT_IOSTRCTRL2_SDIOC(4) | /*CHECK: sdioc might be conditional on unk05cc*/
                LT_IOSTRCTRL2_SDIOD(4);
            reg_hwstrctrl1 |= HW_IOSTRCTRL1_SDIO(5);

    /*  all other Latte boards */
        } else {
            reg_ltstrctrl2 |= LT_IOSTRCTRL2_SDIOB(2) |
                LT_IOSTRCTRL2_SDIOC(4) |
                LT_IOSTRCTRL2_SDIOD(2);
            reg_hwstrctrl1 |= HW_IOSTRCTRL1_SDIO(4);
        }

        *LT_IOSTRCTRL2 = reg_ltstrctrl2; //TODO: 0xfffc0e3f
        *(uint32_t*)0x0d8005cc = reg_unk05cc & ~6; //TODO unknown stuff
    } else return BSP_RVAL_UNKNOWN_HARDWARE_VERSION;

    *HW_IOSTRCTRL1 = reg_hwstrctrl1;
    return BSP_RVAL_OK;
}
