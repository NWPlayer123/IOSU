#pragma once

extern BC_CONFIG bspBoardConfig; //.bss:E604798C
extern BSP_HARDWARE_VERSION bspHardwareVersion; //.bss:E6047984

void udelay_realtime(uint32_t usec);
