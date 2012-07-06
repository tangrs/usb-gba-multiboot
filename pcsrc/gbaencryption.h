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

struct gbaCrcState {
    unsigned crc, rr, hh, xx;
};

struct gbaEncryptionState {
    unsigned seed, kk;
};

void gbaCrcInit(unsigned hh, unsigned rr, int mode, struct gbaCrcState *crc);
void gbaCrcAdd(unsigned data, struct gbaCrcState *state);
unsigned gbaCrcFinalize(unsigned data, struct gbaCrcState *state);
void gbaEncryptionInit(unsigned seed, int mode, struct gbaEncryptionState *state);
unsigned gbaEncrypt(unsigned data, unsigned ptr, struct gbaEncryptionState *state);
