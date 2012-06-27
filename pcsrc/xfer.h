struct gbaHandle;
typedef struct gbaHandle {
    int fd;
    int (*xfer16)(unsigned*,struct gbaHandle*);
} gbaHandle;

gbaHandle* initGbaHandle(char* device, int mode);
void freeGbaHandle(gbaHandle* handle);
int xferGbaInt16Multiplayer(unsigned *data_, gbaHandle* handle);
int xferGbaInt32Normal(unsigned *data_, gbaHandle* handle);
int xferGbaInt16Normal(unsigned *data_, gbaHandle* handle);

enum {
    MODE_MULTIPLAYER,
    MODE_NORMAL
};