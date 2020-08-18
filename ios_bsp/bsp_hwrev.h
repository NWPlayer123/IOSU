/*  Info largely from wiiubrew - thanks Hykem!
    see https://wiiubrew.org/wiki/Hardware/Latte_Registers#LT_ASICREV_ACR
    Info also from cafe2wii DWARF info - thanks Nintendo!
*/
#ifndef _BSP_HWREV_H_
#define _BSP_HWREV_H_

#include <bsp_entity.h>
#include <bc.h>

#include <stdint.h>
#include <stdbool.h>

BSP_RVAL bspMethodGetHardwareVersion(BSP_HARDWARE_VERSION *version);
BSP_RVAL determineWoodBasedHardwareVersion(BSP_HARDWARE_VERSION* version, bool fullCheck);
BSP_RVAL determineLatteBasedHardwareVersion(BSP_HARDWARE_VERSION* version);

BSP_RVAL bspGetConsoleMask(int32_t* mask);

BSP_RVAL bspGetConsoleType(int32_t* consoleType);

BSP_RVAL bspReadBoardConfig(BC_CONFIG* config);

#endif //_BSP_HWREV_H_
