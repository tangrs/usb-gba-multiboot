/*
    This file is part of the USB-GBA multiboot project.

    The USB-GBA multiboot project is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The USB-GBA multiboot project is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the USB-GBA multiboot project.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gbaencryption.h"
#include "xfer.h"

void gbaCrcInit(unsigned hh, unsigned rr, int mode, struct gbaCrcState *crc) {
    crc->hh = hh;
    crc->rr = rr;

    if (mode == MODE_MULTIPLAYER) {
        crc->xx = 0xa517;
        crc->crc = 0xfff8;
    }else{
        crc->xx = 0xc37b;
        crc->crc = 0xc387;
    }
}

void gbaCrcAdd(unsigned data, struct gbaCrcState *state) {
    int bit;
    for (bit = 0 ; bit < 32 ; bit++)
    {
        unsigned tmp = state->crc ^ data;
        state->crc >>= 1;
        data >>= 1;
        if (tmp & 0x01) state->crc ^= state->xx;
    }
}

unsigned gbaCrcFinalize(unsigned data, struct gbaCrcState *state) {
    unsigned crctemp = ((((data&0xFF00)+state->rr)<<8)|0xFFFF0000)+state->hh;
    gbaCrcAdd(crctemp, state);

    return state->crc;
}

void gbaEncryptionInit(unsigned seed, int mode, struct gbaEncryptionState *state) {
    state->seed = seed;

    if (mode == MODE_MULTIPLAYER) {
        state->kk = 0x6465646f;
    }else{
        state->kk = 0x43202F2F;
    }
}

unsigned gbaEncrypt(unsigned data, unsigned ptr, struct gbaEncryptionState *state) {
    ptr = ~(ptr+0x02000000)+1;
    state->seed = (state->seed * 0x6F646573)+1;

    data = (state->seed ^ data)^(ptr ^ state->kk);
    return data;
}
