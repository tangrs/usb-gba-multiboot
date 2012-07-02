int gbaMultibootSend(unsigned char* rom, unsigned length, gbaHandle* gba);
int gbaReady(gbaHandle* gba);
int gbaSendHeader(unsigned char* header, gbaHandle* gba);
int gbaSendMainBlock(unsigned char* block, unsigned length, gbaHandle* gba);