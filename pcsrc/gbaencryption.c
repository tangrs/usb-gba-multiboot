#include "gbaencryption.h"

#define kk 0x6465646f
#define xx 0xa517
#define mm 0xfff8

void gbaCrcInit(unsigned hh, unsigned rr, struct gbaCrcState *crc) {
    crc->hh = hh;
    crc->rr = rr;
    crc->crc = mm;
}

void gbaCrcAdd(unsigned data, struct gbaCrcState *state) {
    int bit;
    for (bit = 0 ; bit < 32 ; bit++)
    {
        unsigned tmp = state->crc ^ data;
        state->crc >>= 1;
        data >>= 1;
        if (tmp & 0x01) state->crc ^= xx;
    }
}

unsigned gbaCrcFinalize(unsigned data, struct gbaCrcState *state) {
    unsigned crctemp = ((((data&0xFF00)+state->rr)<<8)|0xFFFF0000)+state->hh;
    gbaCrcAdd(crctemp, state);

    return state->crc;
}

void gbaEncryptionInit(unsigned seed, struct gbaEncryptionState *state) {
    state->seed = seed;
}

unsigned gbaEncrypt(unsigned data, unsigned ptr, struct gbaEncryptionState *state) {
    ptr = ~(ptr+0x02000000)+1;
    state->seed = (state->seed * 0x6F646573)+1;

    data = (state->seed ^ data)^(ptr ^ kk);
    return data;
}
