#include <gctypes.h>

static uint32_t s_crc32Value;

void crc32MakeTable() {}
void crc32Reset() {
    s_crc32Value = 0;
}

uint32_t crc32Input(void* data, size_t size) {
    s_crc32Value = ~s_crc32Value;

    uint8_t* bytes = (uint8_t*)data;
    for (int i = 0; i < size; i++) {
        uint8_t nLookupIndex = (s_crc32Value ^ bytes[i]) & 0xFF;

        uint32_t crc_constant = nLookupIndex;
        for (int j = 0; j < 8; j++) {
            if (crc_constant & 1) {
                crc_constant >>= 1;
                crc_constant ^= 0xedb88320;
            } else {
                crc_constant >>= 1;
            }
        }

        s_crc32Value = crc_constant ^ (s_crc32Value >> 8);
    }

    return ~s_crc32Value;
}
