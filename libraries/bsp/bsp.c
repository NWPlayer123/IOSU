#include <bsp.h>

/* .kernel.data:081441d4
 * .net.data2:124c5294
 * .acp.data2:e0122640
 * .nimboss.data2:e22ca5f4
 * .bsp.data:e6046984
 */
BSP_HARDWARE_INFO bspHwInfo = {
    .clock = {
        .systemClockFrequency = 248625000,
        .timerFrequency = 1942382,
    },
    .hwver = BSP_HARDWARE_VERSION_LATTE_A4X_EV,
};
