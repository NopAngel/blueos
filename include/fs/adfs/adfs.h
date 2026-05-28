/* fs/adfs/adfs.h */

#define ADFS_DISCRECORD_OFFSET 0x1c0

struct adfs_discrecord {
    uint8_t  log2secsize;    
    uint8_t  secspertrack;   
    uint8_t  heads;       
    uint8_t  density;       
    uint8_t  idlen;        
    uint8_t  log2bpmb;     
    uint8_t  skew;        
    uint8_t  bootopt;        
    uint8_t  reserved[5];
    uint32_t disc_size;     
    uint16_t disc_id;        
    char     disc_name[10];  
} __attribute__((packed));

struct adfs_dir_header {
    uint32_t start_masic;    
    char     name[10];     
};

struct adfs_direntry {
    char     name[10];       
    uint32_t load_addr;      
    uint32_t exec_addr;      
    uint32_t length;         
    uint32_t address;        
    uint8_t  attr;          
} __attribute__((packed));