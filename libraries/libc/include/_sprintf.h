#pragma once

#include <gctypes.h>
#include <stdarg.h>

int vsnprintf(char* str, size_t size, const char* format, va_list ap);
int snprintf(char* str, size_t size, const char* format, ...);

extern void (*printf_callback)(const char* msg);
int vprintf(const char* format, va_list ap);
int printf(const char* format, ...);
