#include "../libburn/crc.h"

unsigned short crc_ccitt(unsigned char *q, int len)
{
    unsigned short crc = 0;
    while (len-- > 0) {
        crc ^= (unsigned short)(*q++) << 8;
        for (int i=0;i<8;i++) crc = (crc & 0x8000) ? (unsigned short)((crc<<1)^0x1021) : (unsigned short)(crc<<1);
    }
    return (unsigned short)~crc;
}

unsigned int crc_32(unsigned char *data, int len)
{
    unsigned int crc = 0;
    while (len-- > 0) {
        crc ^= *data++;
        for (int i=0;i<8;i++) crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320U : crc >> 1;
    }
    return crc;
}
