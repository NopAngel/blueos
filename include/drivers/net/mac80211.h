#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t frame_control;
    uint16_t duration;
    uint8_t  addr1[6];
    uint8_t  addr2[6]; 
    uint8_t  addr3[6]; 
    uint16_t seq_control;
} wifi_mgmt_header;
#pragma pack(pop)