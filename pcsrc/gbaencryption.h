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
