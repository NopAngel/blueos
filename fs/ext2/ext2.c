#include <fs/ext2.h>
#include <kernel/io.h> 
#include <lib/string.h> 
#include <kernel/printk.h>
#include <kernel/colors.h> 

static ext2_superblock_t sb;
static uint32_t block_size;

static void disk_read_block(uint32_t block, uint8_t *buf) {
    uint32_t lba = block * (block_size / 512);
    uint32_t sectors = block_size / 512;

    while (inb(0x1F7) & 0x80); // Wait BSY
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, sectors);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20); // Read command

    uint16_t *ptr = (uint16_t *)buf;
    for (uint32_t i = 0; i < (block_size / 2); i++) {
        while (!(inb(0x1F7) & 0x08)); // Wait DRQ
        ptr[i] = inw(0x1F0);
    }
}

ext2_inode_t* ext2_get_inode(uint32_t inode_num) {
    static uint8_t inode_table_buf[1024];
    static ext2_inode_t inode;

    uint8_t gd_buf[1024];
    disk_read_block(2, gd_buf);
    ext2_group_desc_t *gd = (ext2_group_desc_t*)gd_buf;

    uint32_t index = (inode_num - 1) % sb.inodes_per_group;
    uint32_t block = gd->inode_table + (index * sizeof(ext2_inode_t)) / block_size;
    uint32_t offset = (index * sizeof(ext2_inode_t)) % block_size;

    disk_read_block(block, inode_table_buf);
    memcpy(&inode, inode_table_buf + offset, sizeof(ext2_inode_t));
    return &inode;
}

void ext2_init() {
    block_size = 1024; 
    static uint8_t sb_buffer[1024];
    disk_read_block(1, sb_buffer);
    memcpy(&sb, sb_buffer, sizeof(ext2_superblock_t));

    if (sb.magic != EXT2_MAGIC) {
        printk(RED, "BlueEXT: Magic 0x%x invalid!\n", sb.magic);
        return;
    }
    block_size = 1024 << sb.log_block_size;
    printk(GREEN, "BlueEXT: Ready. Block size: %d bytes\n", block_size);
}

void ext2_ls(uint32_t inode_num) {
    ext2_inode_t *inode = ext2_get_inode(inode_num);
    uint8_t data[1024];
    disk_read_block(inode->block[0], data);

    ext2_dir_entry_t *entry = (ext2_dir_entry_t*)data;
    uint32_t total_read = 0;

    while (total_read < inode->size) {
        char name[256];
        memcpy(name, entry->name, entry->name_len);
        name[entry->name_len] = '\0';

        if (entry->inode > 0) {
            printk(WHITE, " [%s] - Inode: %d\n", name, entry->inode);
        }

        total_read += entry->rec_len;
        entry = (ext2_dir_entry_t*)((uint8_t*)entry + entry->rec_len);
        if (entry->rec_len == 0) break;
    }
}

void ext2_read_file(uint32_t inode_num) {
    ext2_inode_t *inode = ext2_get_inode(inode_num);
    uint8_t data[1024];

    disk_read_block(inode->block[0], data);
    
    printk(YELLOW, "File content:\n");
    for(uint32_t i=0; i < inode->size && i < 1024; i++) {
        printk(WHITE, "%c", data[i]);
    }
    printk(WHITE, "\n");
}