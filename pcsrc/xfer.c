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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <poll.h>
#include <errno.h>
#include "xfer.h"

#include <stdio.h>

gbaHandle* initGbaHandle(char* device, int mode) {
    gbaHandle *gba = malloc(sizeof(gbaHandle));
    if (!gba) return NULL;

    if ( (gba->fd = open(device, O_RDWR | O_NDELAY | O_SYNC | O_NOCTTY)) < 0 ) {
        goto error;
    }
    struct termios tio = {0};
    tcgetattr(gba->fd, &tio);

    if (mode == MODE_MULTIPLAYER) {
        tio.c_cc[VMIN] = 2;
        gba->xfer16 = xferGbaInt16Multiplayer;
        gba->xfer32 = NULL;
    }else if (mode == MODE_NORMAL) {
        tio.c_cc[VMIN] = 4;
        gba->xfer16 = xferGbaInt16Normal;
        gba->xfer32 = xferGbaInt32Normal;
    }else{
        goto error;
    }

    tio.c_cc[VTIME] = 5;
    tio.c_oflag &= ~OPOST;
    tio.c_cflag |= CLOCAL | CS8 | CREAD;
    if (tcsetattr(gba->fd,TCSANOW,&tio)) {
        goto error;
    }

    gba->mode = mode;

    finish:
    return gba;
    error:
    if (gba->fd >= 0) close(gba->fd);
    free(gba);
    gba = NULL;
    goto finish;
}


void freeGbaHandle(gbaHandle* handle) {
    close(handle->fd);
    free(handle);
}

int xferGbaInt16Multiplayer(unsigned *data_, gbaHandle* handle) {
    unsigned char c[2];
    unsigned data = *data_;
    struct pollfd polldata = {
        .fd = handle->fd,
        .events = POLLIN,
        .revents = 0
    };

    c[0] = (data>>8) & 0xff;
    c[1] = data & 0xff;
    write(handle->fd, &c, 2);

    data = 0;
    poll(&polldata, 1, 50);
    if (read(handle->fd, &c, 2) != 2) return -1;
    data = (c[0]<<8) | c[1];

    *data_ = data;
    return 0;
}

int xferGbaInt32Normal(unsigned *data_, gbaHandle* handle) {
    unsigned char c[4];
    unsigned data = *data_;
    struct pollfd polldata = {
        .fd = handle->fd,
        .events = POLLIN,
        .revents = 0
    };

    c[0] = (data>>24) & 0xff;
    c[1] = (data>>16) & 0xff;
    c[2] = (data>>8) & 0xff;
    c[3] = data & 0xff;
    write(handle->fd, &c, 4);

    data = 0;
    poll(&polldata, 1, 50);
    if (read(handle->fd, &c, 4) != 4) return -1;
    data = (c[0]<<24) | (c[1]<<16) | (c[2]<<8) | c[3];

    *data_ = data;
    return 0;
}

int xferGbaInt16Normal(unsigned *data_, gbaHandle* handle) {
    (*data_) &= 0xffff;
    int ret = xferGbaInt32Normal(data_, handle);
    (*data_) >>= 16;
    return ret;
}
