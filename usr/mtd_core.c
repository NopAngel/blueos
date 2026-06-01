#include <kernel/printk.h>

struct mtd_info {
    const char* name;
    uint32_t size;
    uint32_t erasesize;
};

void mtd_register_device(struct mtd_info* mtd) {
    if (!mtd) return;
    printk("[  MTD  ] Found flash device: %s (%d KB)\n", 
           mtd->name, mtd->size / 1024);
}