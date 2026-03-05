#include <stdint.h>
#include <blueos/io.h>

struct prd_entry {
    uint32_t base_addr; 
    uint16_t byte_count;
    uint16_t reserved : 15;
    uint16_t last_entry : 1;
} __attribute__((packed));


struct prd_entry my_prdt[1] __attribute__((aligned(4)));