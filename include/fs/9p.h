/* fs/9p/9p.h */

#define P9_TVERSION 100
#define P9_RVERSION 101
#define P9_TATTACH  104
#define P9_RATTACH  105
#define P9_TWALK    110
#define P9_RWALK    111
#define P9_TOPEN    112
#define P9_ROPEN    113
#define P9_TREAD    116
#define P9_RREAD    117
#define P9_MAX_BUF 8192

enum {
    Tversion = 100,
    Rversion,
    Tauth = 102,
    Rauth,
    Tattach = 104,
    Rattach
};

struct p9_header {
    uint32_t size;   
    uint8_t  type;    
    uint16_t tag;     
} __attribute__((packed));



struct v9p_session {
    uint8_t out_buf[P9_MAX_BUF]; 
    uint8_t in_buf[P9_MAX_BUF];  
    uint32_t msize;              
    uint16_t last_tag;        
    int virtio_id;              
};
struct p9_qid {
    uint8_t  type;      
    uint32_t version;   
    uint64_t path;      
} __attribute__((packed));

typedef struct {
    uint32_t size;
    uint8_t  type; 
    uint16_t tag;  
} __attribute__((packed)) p9_header;