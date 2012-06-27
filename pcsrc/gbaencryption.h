struct gbaCrcState {
    unsigned crc, rr, hh;
};

struct gbaEncryptionState {
    unsigned seed;
};

void gbaCrcInit(unsigned hh, unsigned rr, struct gbaCrcState *crc);
void gbaCrcAdd(unsigned data, struct gbaCrcState *state);
unsigned gbaCrcFinalize(unsigned data, struct gbaCrcState *state);
void gbaEncryptionInit(unsigned seed, struct gbaEncryptionState *state);
unsigned gbaEncrypt(unsigned data, unsigned ptr, struct gbaEncryptionState *state);