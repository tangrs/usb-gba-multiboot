#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "xfer.h"
#include "multi.h"

int main(int argc, char* argv[]) {
    if (argc < 2) return 0;

    gbaHandle *gba = initGbaHandle("/dev/tty.usbmodem12341", MODE_NORMAL);
    FILE *rom = fopen(argv[1],"rb");
    if (!gba || !rom) return -1;
    unsigned size;
    struct stat stats;
    fstat(fileno(rom), &stats);
    size = stats.st_size;
    if (size < 0x60 * 2) return -1;

    size = (size+0xf)&(~0xf); //Align to 16bytes

    unsigned char* romdata = malloc(size);
    fread(romdata, 1, size, rom);

    if (gbaReady(gba)) goto finish;
    if (gbaSendHeader(romdata, gba)) goto finish;
    if (gbaSendMainBlock(romdata, size, gba)) goto finish;


    finish:
    free(romdata);
    fclose(rom);
    freeGbaHandle(gba);
    return 0;

}