#include <stdint.h>

enum {
    Tversion = 100, Rversion,
    Tauth = 102,    Rauth,
    Tattach = 104,  Rattach,
    Twalk = 110,    Rwalk,
    Topen = 112,    Ropen,
    Tread = 116,    Rread,
    Twrite = 118,   Rwrite,
    Tclunk = 120,   Rclunk,
    Tremove = 122,  Rremove,
    Tstat = 124,    Rstat,
};

#pragma pack(push, 1)
typedef struct {
    uint32_t size; 
    uint8_t  type; 
    uint16_t tag;  
} p9_header;
#pragma pack(pop)