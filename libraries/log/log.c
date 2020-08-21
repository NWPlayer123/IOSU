#include "log.h"

#include <gctypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

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

log_fd* log_get_fd(int handle) {
    if (handle < 0x20 && log_fds[handle].active) {
        return &log_fds[handle];
    }
    return NULL;
}

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

/* .bsp.data:e6046980
 */
static int log_syslog_handle = IOS_ERROR_INVALID;

IOSError log_print_syslog(const char* fmt, va_list ap, bool include_ts) {
    if (log_syslog_handle < 0) {
        log_syslog_handle = IOS_Open("/dev/syslog", 0);
        if (log_syslog_handle < 0) {
            return IOS_ERROR_INVALID;
        }
    }

    char* str = IOS_HeapAlloc(0xcaff, 0x120);
    if (str == NULL) {
        return IOS_ERROR_INVALID;
    }

    int strOffset = 0;

    if (include_ts) {
    /*  TODO clean this up */
        IOSTimerTicks uptime;
        IOS_GetUpTime64(&uptime);
        uint64_t msec = uptime / 1000ull;
        uint64_t work = (msec & 0xFFFFFFFF) / 1000;
        uint32_t sec = (int)work;
        work /= 60;
        uint32_t min = (int)work;
        work /= 60;
        uint32_t hrs = (int)work;

        int ret = snprintf(str, 0x100, "%02d:%02d:%02d:%03d: ",
            hrs - hrs/24*24, min - hrs*60, sec - min*60, (int)msec - sec*1000
        );
        if (ret >= 0) {
            strOffset += ret;
        }
    }

    int ret = vsnprintf(&str[strOffset], 0x100-strOffset, fmt, ap);
    if (ret < 1) {
        return IOS_ERROR_INVALID;
    }
    strOffset += ret;

    IOSError err = IOS_Write(log_syslog_handle, str, strOffset);
    IOS_HeapFree(0xcaff, str);

    return err;
}

void _Noreturn log_panic(const char* fmt, ...) {
    char msg[128];
    memset(msg, 0, sizeof(msg));

    va_list ap;
    va_start(ap, fmt);
    int msg_size = vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    IOS_Panic(msg, msg_size);
}

void _log_print_syslog_ts(const char* fmt, va_list ap) {
    log_print_syslog(fmt, ap, true);
}

void log_print_syslog_ts(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    _log_print_syslog_ts(fmt, ap);
    va_end(ap);
}

void log_fatal(int handle, const char* file, const char* function, int line, const char* message, ...) {
    char msg[256];
    memset(msg, 0, 256);

    int msgOffset = 0;

    log_fd* fd = log_get_fd(handle);
    if (fd != NULL) {
        msgOffset += snprintf(msg, 256, "%s ", fd->name);
    }

    int tid = IOS_GetCurrentThreadId();
    IOSProcessId pid = IOS_GetCurrentProcessId();
    msgOffset += snprintf(&msg[msgOffset], 256 - msgOffset,
        "ASSERT: file=%s, function=%s(), line=%d, processID=%d, threadID=%d, ",
        file, function, line, pid, tid
    );

    va_list ap;
    va_start(ap, message);
    vsnprintf(&msg[msgOffset], 256 - msgOffset, message, ap);
    va_end(ap);

    printf("\n%s\n", msg);
    log_print_syslog_ts("\n%s\n", msg);
    delay_ticks(100000);

    log_panic("\n%s\n", msg);
}
