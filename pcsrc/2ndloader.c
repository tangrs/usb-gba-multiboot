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

int secondStageLoad(unsigned char* rom, unsigned size, int xferdelay, gbaHandle *gba) {
    fprintf(stderr,"Waiting for second stage loader\n");
    unsigned data, timeout = 16*1000, i;

    if (xferdelay < 0) xferdelay = 500;

    do {
        data = 0xfa57b007;
        xferGbaInt32Normal(&data, gba);
        usleep(250);
        timeout--;
    } while (data != 0xfa57b007 && timeout);
    if (data != 0xfa57b007) return -1;

    struct timeval timebegin, timeend;
    gettimeofday(&timebegin, NULL);

    data = size>>2;
    xferGbaInt32Normal(&data, gba);
    sleep(1);

    /*for (i=0;i<size;i+=4) {
        data = rom[i] | (rom[i+1]<<8) | (rom[i+2]<<16) | (rom[i+3]<<24);
        xferGbaInt32Normal(&data, gba);
        gettimeofday(&timeend, NULL);
        double elapsed = (timeend.tv_sec + (timeend.tv_usec / 1000000.0))-(timebegin.tv_sec + (timebegin.tv_usec / 1000000.0));
        fprintf(stderr,"\n2nd stage loader (%02d%%): %08x (%.0fB/s)",(i*100)/size, data, i/elapsed);
    }

    if (data == (0x02000000 + i-4)) fprintf(stderr,"\n2nd stage loading successful\n");*/

    unsigned checksum;
    for (i=0;i<size;i+=4) {
        data = (rom[i+3]<<24) | (rom[i+2]<<16) | (rom[i+1]<<8) | rom[i];
        checksum += data;
        xferGbaInt32Normal(&data, gba);

        gettimeofday(&timeend, NULL);
        double elapsed = (timeend.tv_sec + (timeend.tv_usec / 1000000.0))-(timebegin.tv_sec + (timebegin.tv_usec / 1000000.0));
        fprintf(stderr,"\r2nd stage loader (%02d%%): Writing to %08x %.0fB/s",(i*100)/size,data,i/elapsed);
    }

    data = checksum;
    xferGbaInt32Normal(&data, gba);
    fprintf(stderr,"\ndebug: i = %08x, size = %08x",i,size);
    fprintf(stderr,"\n2nd stage loader checksum: Recieved %08x Expected %08x\n",data,checksum);

    if (data == checksum) fprintf(stderr,"2nd stage loading successful\n");

    return 0;
}