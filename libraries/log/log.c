#include "log.h"

#include <gctypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <ios_api/ios.h>

IOSSemaphore log_mutex = -1; //.bsp.data:e604697c

typedef struct log_fd {
    char name[16];
    int unk1;
    bool active;
    int unk2;
} log_fd;
log_fd log_fds[0x20]; //.bsp.bss:e6047e68
int active_log_fds; //.bsp.bss:e6047e64

//.bsp.text: e600e35c
int log_open(const char* name, int unk1, int unk2) {
    IOSError err;

    if (log_mutex < 0) {
        log_mutex = IOS_CreateSemaphore(1, 1);
        if (log_mutex < 0) {
            printf("LOG ERROR: mutex init error!\n"); //.bsp.text:e600ec28
            return -1;
        }
    }

    err = IOS_WaitSemaphore(log_mutex, 0);
    if (err != IOS_ERROR_OK) {
        printf("LOG ERROR: log_open(%s, %d, ...) failed\n", name, unk1);
        return -1;
    }

    int ndx = -1;
    bool found = false;
    for (int i = 0; i < sizeof(log_fds)/sizeof(log_fds[0]); i++) {
    /*  Take note of the first empty slot */
        if (log_fds[i].active == false) {
            if (ndx == -1) {
                ndx = i;
            }
        } else {
            int ret = strncmp(log_fds[i].name, name, sizeof(log_fds[i].name));
            if (ret == 0) {
                found = true;
                ndx = i;
                break;
            }
        }
    }

    if (ndx == -1) {
        IOS_SignalSemaphore(log_mutex);
        printf("LOG ERROR: log_open(%s, %d, ...) failed\n", name, unk1);
        return -1;
    }

    log_fds[ndx].unk1 = unk1;
    log_fds[ndx].unk2 = unk2;

    if (!found) {
        log_fds[ndx].active = true;
        active_log_fds++;
    }

    IOS_SignalSemaphore(log_mutex);
    return ndx;
}
