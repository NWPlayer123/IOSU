#include <_sprintf.h>
#include <ios_api/realview.h>
#include <gctypes.h>
#include <stdarg.h>

typedef struct sprintf_ctx {
    void (*add)(void* ctx, char c);
    void (*flush)(void* ctx);
} sprintf_ctx;

typedef struct sprintf_ctx_buffer {
    sprintf_ctx io;
    char* buffer;
    size_t offset;
    size_t size;
} sprintf_ctx_buffer;

/* .bsp.text:e600ecd4
 */
static void _sprintf_buffer_add(void* ctx_in, char c) {
    sprintf_ctx_buffer* ctx = (sprintf_ctx_buffer*)ctx_in;

    if (ctx->offset < ctx->size) {
        ctx->buffer[ctx->offset] = c;
    }
    ctx->offset++;
}

/* .bsp.text:e600eca8
 */
static void _sprintf_buffer_flush(void* ctx_in) {
    sprintf_ctx_buffer* ctx = (sprintf_ctx_buffer*)ctx_in;

    if (ctx->offset < ctx->size) {
        ctx->buffer[ctx->offset] = '\0';
    } else {
        ctx->buffer[ctx->size - 1] = '\0';
    }
}

/* .bsp.text:e600ed6c
 */
static void _sprintf_cb(sprintf_ctx* io, const char* fmt, va_list ap) {
    //lol nah
    //TODO
}

/* .bsp.text:e600f430
 */
int vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    sprintf_ctx_buffer ctx = {
        .io = {
            .add = _sprintf_buffer_add,
            .flush = _sprintf_buffer_flush,
        },
        .buffer = str,
        .offset = 0,
        .size = size,
    };
    _sprintf_cb(&ctx.io, format, ap);
    return ctx.offset;
}

/* .bsp.text:e600f47c
 */
int snprintf(char* str, size_t size, const char* format, ...) {
    va_list ap;
    size_t ret;

    va_start(ap, format);
    ret = vsnprintf(str, size, format, ap);
    va_end(ap);

    return ret;
}

typedef struct sprintf_ctx_realview {
    sprintf_ctx io;
    char buffer[16];
    size_t offset;
    size_t written;
} sprintf_ctx_realview;

/* .bsp.text:e600eba8
 */
static void _sprintf_realview_add(void* ctx_in, char c) {
    sprintf_ctx_realview* ctx = (sprintf_ctx_realview*)ctx_in;

    ctx->buffer[ctx->offset] = c;
    ctx->written++;
    ctx->offset++;
    if (ctx->offset == 15) {
        ctx->io.flush(ctx_in);
    }
}

/* .bsp.data:e60481e8
 */
void (*printf_callback)(const char* msg);

/* .bsp.text:e600ec58
 */
static void _sprintf_realview_flush(void* ctx_in) {
    sprintf_ctx_realview* ctx = (sprintf_ctx_realview*)ctx_in;

    if (ctx->offset == 0) return;

    ctx->buffer[ctx->offset] = '\0';
    ctx->offset = 0;

    if (printf_callback) {
        printf_callback(ctx->buffer);
    } else {
        __sys_write0(ctx->buffer);
    }
}

/* .bsp.text:e600ebe4
 */
int vprintf(const char* format, va_list ap) {
    sprintf_ctx_realview ctx = {
        .io = {
            .add = _sprintf_realview_add,
            .flush = _sprintf_realview_flush,
        },
        .offset = 0,
        .written = 0,
    };
    _sprintf_cb(&ctx.io, format, ap);
    return ctx.offset;
}

/* .bsp.text:e600ec28
 */
int printf(const char* format, ...) {
    va_list ap;
    size_t ret;

    va_start(ap, format);
    ret = vprintf(format, ap);
    va_end(ap);

    return ret;
}
