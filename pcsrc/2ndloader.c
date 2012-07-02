#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include "xfer.h"
#include "multi.h"
#include "loader.h"


int secondStageSend(gbaHandle *gba) {
    if (gba->mode != MODE_NORMAL) return -1;
    fprintf(stderr,"Sending second stage loader\n");
    return gbaMultibootSend(loaderbin, sizeof(loaderbin), gba);
}

int secondStageLoad(unsigned char* rom, unsigned size, gbaHandle *gba) {
    fprintf(stderr,"Waiting for second stage loader\n");
    unsigned data, timeout = 16, i;

    do {
        data = 0xfa57b007;
        xferGbaInt32Normal(&data, gba);
        usleep(250*1000);
        timeout--;
    } while (data != 0xfa57b007 && timeout);
    if (data != 0xfa57b007) return -1;

    struct timeval timebegin, timeend;
    gettimeofday(&timebegin, NULL);

    data = size>>2;
    xferGbaInt32Normal(&data, gba);
    for (i=0;i<size;i+=4) {
        data = rom[i] | (rom[i+1]<<8) | (rom[i+2]<<16) | (rom[i+3]<<24);
        xferGbaInt32Normal(&data, gba);
        gettimeofday(&timeend, NULL);
        double elapsed = (timeend.tv_sec + (timeend.tv_usec / 1000000.0))-(timebegin.tv_sec + (timebegin.tv_usec / 1000000.0));
        fprintf(stderr,"\r2nd stage loader (%02d%%): %08x (%.0fB/s)",(i*100)/size, data, i/elapsed);
    }

    if (data == (0x02000000 + i-4)) fprintf(stderr,"\n2nd stage loading successful\n");

    return 0;
}