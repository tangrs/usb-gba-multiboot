#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "xfer.h"
#include "2ndloader.h"
#include "multi.h"

int main(int argc, char* argv[]) {
    if (argc < 2) return 0;
    struct timeval timebegin, timeend;
    gettimeofday(&timebegin, NULL);


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

    if (secondStageSend(gba) == 0) {
        secondStageLoad(romdata, size, gba);
    }

    free(romdata);
    fclose(rom);
    freeGbaHandle(gba);

    gettimeofday(&timeend, NULL);
    double executiontime = timeend.tv_sec + (timeend.tv_usec / 1000000.0);
    executiontime -= timebegin.tv_sec + (timebegin.tv_usec / 1000000.0);
    fprintf(stderr,"\nTotal transfer time: %.2f seconds\n", executiontime);
    return 0;

}