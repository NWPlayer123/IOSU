#include <delay.h>

//.text:e60005b0
void udelay_realtime(uint32_t usec) {
    uint32_t ticks = usec_to_timer_ticks(usec);
    uint32_t start = *LT_TIMER;
    uint32_t now;
    do {
        now = *LT_TIMER;
    } while (now - start < ticks);
}
