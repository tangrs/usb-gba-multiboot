#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "xfer.h"
#include "2ndloader.h"
#include "multi.h"

enum {
    SEND_2ND_LOADER = (1<<0),
    SEND_VIA_2ND_LOADER = (1<<1)
};


void printHelp() {
    fprintf(stderr,
    "Usage: gbaxfer [-m] [-2 | -c] /path/to/file\n"
    "    -2: Send 2nd loader\n"
    "    -c: Boot ROM from 2nd loader\n");
}


unsigned processOptions(int argc, char* argv[], char ** romfile) {
    unsigned options = 0, i;
    *romfile = NULL;
    for (i=1; i<argc;i++) {
        if (*argv[i] == '-') {
            switch (*(argv[i]+1)) {
                case '2':
                    options |= SEND_2ND_LOADER;
                    break;
                case 'b':
                    options |= SEND_VIA_2ND_LOADER;
                    break;
                default:
                    fprintf(stderr,"Unknown option: -%s\n", argv[i]);
                    break;
            }
        } else if (i == argc-1) {
            *romfile = argv[i];
        }
    }
    return options;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 0;
    }
    char *filename;
    unsigned options = processOptions(argc, argv, &filename);

    struct timeval timebegin, timeend;
    gettimeofday(&timebegin, NULL);


    gbaHandle *gba = initGbaHandle("/dev/tty.usbmodem12341", MODE_NORMAL);

    int ret = 0;
    if (options & SEND_2ND_LOADER) {
        ret = secondStageSend(gba);
    }
    if (filename) {
        FILE *rom = fopen(filename,"rb");
        if (!gba || !rom) return -1;
        unsigned size;
        struct stat stats;
        fstat(fileno(rom), &stats);
        size = stats.st_size;
        if (size < 0x60 * 2) return -1;
        size = (size+0xf)&(~0xf); //Align to 16bytes

        unsigned char* romdata = malloc(size);
        fread(romdata, 1, size, rom);

        if ((options & SEND_VIA_2ND_LOADER) && ret == 0) {
            secondStageLoad(romdata, size, -1, gba);
        }
        if ((options & (SEND_2ND_LOADER | SEND_VIA_2ND_LOADER)) == 0) {
            gbaMultibootSend(romdata, size, gba);
        }

        free(romdata);
        fclose(rom);
    }

    freeGbaHandle(gba);

    gettimeofday(&timeend, NULL);
    double executiontime = timeend.tv_sec + (timeend.tv_usec / 1000000.0);
    executiontime -= timebegin.tv_sec + (timebegin.tv_usec / 1000000.0);
    fprintf(stderr,"\nTotal transfer time: %.2f seconds\n", executiontime);
    return 0;

}