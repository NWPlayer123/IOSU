#include <bsp.h>
#include <ios_api/ios.h>
#include <gctypes.h>

uint32_t usec_to_timer_ticks(uint32_t usec) {
    uint64_t ticks64 = ((uint64_t)bspHwInfo.clock.timerFrequency * (uint64_t)usec) / 1000000;
    if (ticks64 >> 32) {
        return UINT_MAX;
    }

    uint32_t ticks = (uint32_t)ticks64;

    if (usec && !ticks) {
        return 1;
    }
    return ticks;
}

void udelay(uint32_t usec) {
    uint32_t ticks = usec_to_timer_ticks(usec);
    uint32_t start = IOS_GetUpTime();
    uint32_t now;
    do {
        now = IOS_GetUpTime();
    } while (now - start < ticks);
}

//.bsp.text:e600f5d0
IOSError delay_ticks(uint32_t ticks) {
    IOSMessage msg_mem;
    IOSMessageQueue queue = IOS_CreateMessageQueue(&msg_mem, 1);
    if (queue < 0) return queue;

    IOSTimer timer = IOS_CreateTimer(ticks, 0, queue, (IOSMessage*)0xbabecafe);
    if (timer < 0) {
        IOS_DestroyMessageQueue(queue);
        return timer;
    }

    IOSMessage* msg;
    IOS_ReceiveMessage(queue, &msg, 0);

    IOS_DestroyTimer(timer);
    IOS_DestroyMessageQueue(queue);
    return IOS_ERROR_OK;
}
