#include <stdint.h>
#include <fs/9p.h>

void p9_send_version(uint32_t msize) {

    uint8_t buffer[64];
    p9_header *h = (p9_header*)buffer;
    
    h->type = Tversion;
    h->tag = 0xFFFF; 
    
    uint32_t *ms = (uint32_t*)(buffer + 7);
    *ms = msize;
    
    uint16_t *vlen = (uint16_t*)(buffer + 11);
    *vlen = 6;
    char *vstr = (char*)(buffer + 13);
    for(int i=0; i<6; i++) vstr[i] = "9P2000"[i];
    
    h->size = 7 + 4 + 2 + 6;
    
}

