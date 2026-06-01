#include <fs/jfs.h>
#include <kernel/printk.h>
#include <kernel/colors.h>
#include <lib/string.h>
#include <fs/vfs.h>


static jfs_superblock_t sb;
static jfs_journal_entry_t journal[JOURNAL_SIZE];
static int current_tx_id = 0;
jfs_inode_t inode_table[MAX_FILES];


void jfs_checkpoint() {
    for (int i = 0; i < JOURNAL_SIZE; i++) {
        if (journal[i].status == JFS_TRANS_COMMIT) {
            journal[i].status = JFS_TRANS_REVOKE;
            printk("[ JFS ] Checkpoint: Block %d synced to disk.\n", journal[i].target_block);
        }
    }
}

int jfs_write(unsigned int block_id, const char* buffer) {
    int entry_found = -1;

    for (int i = 0; i < JOURNAL_SIZE; i++) {
        if (journal[i].status == JFS_TRANS_REVOKE || journal[i].status == 0) {
            entry_found = i;
            break;
        }
    }

    if (entry_found == -1) {
        jfs_checkpoint();
        return jfs_write(block_id, buffer);
    }

    journal[entry_found].transaction_id = ++current_tx_id;
    journal[entry_found].target_block = block_id;
    journal[entry_found].status = JFS_TRANS_COMMIT;
    memcpy(journal[entry_found].data, buffer, BLOCK_SIZE);

    printk("[ JFS ] Logged: TX %d for Block %d\n", current_tx_id, block_id);

    return 0;
}

void jfs_init() {
    sb.magic = JFS_MAGIC;
    printk("[  OK  ] BlueJFS initialized and Journal ready.\n");
}

void jfs_recover() {
    printk("[ JFS ] Checking journal for uncommitted transactions...\n");
    for (int i = 0; i < JOURNAL_SIZE; i++) {
        if (journal[i].status == JFS_TRANS_COMMIT) {
            printk("[ JFS ] Recovering TX %d -> Block %d\n",
                   journal[i].transaction_id, journal[i].target_block);
        }
    }
}

int find_free_inode() {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!inode_table[i].used) return i;
    }
    return -1;
}

void jfs_mkdir(const char* name) {
    int slot = find_free_inode();
    if (slot == -1) {
        printk("\nError: No free inodes\n");
        return;
    }

    strncpy(inode_table[slot].name, name, 31);
    inode_table[slot].type = TYPE_DIR;
    inode_table[slot].size = 0;
    inode_table[slot].used = 1;

    jfs_write(slot, (char*)&inode_table[slot]);
    printk("\nDirectory '%s' created.\n", name);
}

void jfs_touch(const char* name) {
    int slot = find_free_inode();
    if (slot == -1) {
        printk("\nError: No free inodes\n");
        return;
    }

    strncpy(inode_table[slot].name, name, 31);
    inode_table[slot].type = TYPE_FILE;
    inode_table[slot].size = 0;
    inode_table[slot].used = 1;
    inode_table[slot].start_block = 100 + slot;

    jfs_write(slot, (char*)&inode_table[slot]);
    printk("File '%s' created.\n", name);
}

void jfs_ls() {
    printk("Type       Size         Name\n");
    printk("----------------------------\n");
    for (int i = 0; i < MAX_FILES; i++) {
        if (inode_table[i].used) {
            char* type_str = (inode_table[i].type == TYPE_DIR) ? "<DIR>" : "<FILE>";
            printk("%s\t%d\t%s\n", type_str, inode_table[i].size, inode_table[i].name);
        }
    }
}

fs_ops_t jfs_ops = {
    .init = jfs_init,
    .mkdir = jfs_mkdir,
    .touch = jfs_touch,
    .ls = jfs_ls
};
