#pragma once
#include <stdint.h>
#include <ios_api/ios.h>

uint32_t usec_to_timer_ticks(uint32_t usec);
void udelay(uint32_t usec);
IOSError delay_ticks(uint32_t ticks);
