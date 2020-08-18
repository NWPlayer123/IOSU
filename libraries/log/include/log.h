#pragma once

int log_open(const char* handle, int unk1, int unk2);
int log_debug_printf(const char* fmt, ...);
void log_fatal(int handle, const char* file, const char* function, int line, const char* msg, ...); //.bsp.text:e600e264
