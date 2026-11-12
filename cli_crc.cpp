
#include <stdint.h>

unsigned short crc16(uint8_t* buf, int len)
{
    unsigned short crc = 0xffff;

    for (int i = 0; i < len; i++) {
        crc ^= buf[i];

        for (int bit = 0; bit < 8; bit++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0x8408;
            else
                crc >>= 1;
        }
    }

    return (~crc) & 0xffff;
}
