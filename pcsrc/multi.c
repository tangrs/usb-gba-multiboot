#include <stdio.h>
#include <unistd.h>
#include "xfer.h"
#include "gbaencryption.h"

#define masterwaittime ((1000000/16))

#define GBA_READY_CMD 0x6202
#define GBA_READY_REPLY 0x7202
#define GBA_EXCH_MS_INFO_CMD 0x6100

struct gbaCrcEncryptionPair {
    unsigned init;
    struct gbaCrcState crc;
    struct gbaEncryptionState encryption;
};

int gbaReady(gbaHandle* gba) {
    unsigned data, timeout = 33;
    do {
        data = GBA_READY_CMD;
        gba->xfer16(&data, gba);
        fprintf(stderr,"Ready: Recieved %04x\n",data);
        timeout--;
        usleep(masterwaittime);
    } while (data != GBA_READY_REPLY && timeout);

    if (data != GBA_READY_REPLY) return -1;
    return 0;
}

int gbaSendHeader(unsigned char* header, gbaHandle* gba) {
    unsigned data = GBA_EXCH_MS_INFO_CMD, i;
    gba->xfer16(&data, gba);

    for (i=0; i<0x60;i++) {
        data = (header[i*2]) | (header[(i*2) + 1]<<8);
        gba->xfer16(&data, gba);
        fprintf(stderr,"\rHeader (%02d%%): Recieved %04x",(i*100)/0x60,data);
    }

    data = 0x6200;
    gba->xfer16(&data, gba);
    fprintf(stderr,"\rHeader complete: Recieved %04x\n",data);

    return 0;
}

static struct gbaCrcEncryptionPair gbaGetKeys(unsigned length, gbaHandle* gba) {
    struct gbaCrcEncryptionPair state = {0};
    unsigned data, encryptionseed, hh, rr;

    data = 0x63c1;
    gba->xfer16(&data, gba);
    fprintf(stderr,"Send encryption key: Recieved %04x\n",data);

    data = 0x63c1;
    gba->xfer16(&data, gba);
    fprintf(stderr,"Get encryption key: Recieved %04x\n",data);
    if (!((data >> 8) == 0x73)) return state;


    encryptionseed = ((data&0x0FF)<<8)|0x0FFFF00C1;
    hh = ((data&0x0FF)+0x20F) & 0xff;

    data = 0x6400 | hh;
    gba->xfer16(&data, gba);
    fprintf(stderr,"Encryption confirmation: Recieved %04x\n",data);
    usleep(masterwaittime);

    data = ((length-0xC0)>>2)-0x34;
    gba->xfer16(&data, gba);
    fprintf(stderr,"Length information exchange: Recieved %04x\n",data);
    rr = data & 0xff;

    gbaCrcInit(hh, rr, &state.crc);
    gbaEncryptionInit(encryptionseed, &state.encryption);
    state.init = 1;

    return state;
}

int gbaSendMainBlock(unsigned char* rom, unsigned length, gbaHandle* gba) {
    unsigned data, timeout;
    length &= ~(0xf); //Align to 16 bytes

    data = 0x6202;
    gba->xfer16(&data, gba);
    fprintf(stderr,"Sending commands: Recieved %04x\n",data);

    struct gbaCrcEncryptionPair crcenc = gbaGetKeys(length, gba);
    if (!crcenc.init) return -1;

    unsigned clientptr;
    for (clientptr=0xc0;clientptr<length;clientptr+=4) {
        unsigned data32 = rom[clientptr] | (rom[clientptr+1]<<8) | (rom[clientptr+2]<<16) | (rom[clientptr+3]<<24);

        gbaCrcAdd(data32, &crcenc.crc);
        data32 = gbaEncrypt(data32, clientptr, &crcenc.encryption);

        data = data32;
        gba->xfer16(&data, gba);
        fprintf(stderr,"\rMain block (%02d%%): Recieved %04x",(clientptr*100)/length,data);
        if (data != (clientptr&0xffff)) {
            fprintf(stderr,"\nTransmission error");
            return -1;
        }

        data = data32>>16;
        gba->xfer16(&data, gba);
        fprintf(stderr,"\rMain block (%02d%%): Recieved %04x",((clientptr+2)*100)/length,data);
        if (data != ((clientptr+2)&0xffff)) {
            fprintf(stderr,"\nTransmission error");
            return -1;
        }
    }

    data = 0x0065;
    gba->xfer16(&data, gba);
    fprintf(stderr,"\rMain block Complete: Recieved %04x\n",data);

    timeout = 33;
    do {
        data = 0x0065;
        gba->xfer16(&data, gba);
        fprintf(stderr,"\rChecksum wait: Recieved %04x",data);
        timeout--;
        usleep(masterwaittime);
    } while (data != 0x0075 && timeout);
    if (data != 0x0075) return -1;

    data = 0x0066;
    gba->xfer16(&data, gba);
    fprintf(stderr,"\nChecksum signal: Recieved %04x\n",data);
    gbaCrcFinalize(data, &crcenc.crc);

    data = crcenc.crc.crc;
    gba->xfer16(&data, gba);
    fprintf(stderr,"Checksum: Recieved %04x Expected %04x\n",data, crcenc.crc.crc&0xffff);

    return 0;
}