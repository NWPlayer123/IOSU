#pragma once

#include <gctypes.h>

void crc32MakeTable();
void crc32Reset();

uint32_t crc32Input(void* data, size_t size);
