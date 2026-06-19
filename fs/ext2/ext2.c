#include <drivers/disk.h>
#include <fs/ext2.h>
#include <fs/vfs.h>
#include <kernel/malloc.h>
#include <kernel/printk.h>
#include <lib/string.h>
#include <mm/memory.h>

static ext2_superblock_t *sb = NULL;
static uint32_t current_dir_inode = 2;
static uint32_t ext2_base_lba = 0;

/* --- Helpers de Bloques --- */

void ext2_set_base_lba(uint32_t base_lba) { ext2_base_lba = base_lba; }

void ext2_read_block(uint32_t block, void *buf) {
  disk_read(ext2_base_lba + block * (BLOCK_SIZE / 512), (uint8_t *)buf,
            BLOCK_SIZE / 512);
}

void ext2_write_block(uint32_t block, void *buf) {
  disk_write(ext2_base_lba + block * (BLOCK_SIZE / 512), (uint8_t *)buf,
             BLOCK_SIZE / 512);
}
/* --- Gestión de Recursos (Bitmaps) --- */

bool ext2_init() {
  boot_msg("FS", "Initializing EXT2 filesystem...\n", 0);
  sb = (ext2_superblock_t *)kmalloc(sizeof(ext2_superblock_t));

  if (!sb) {
    boot_msg("EXT2", "OOM for Superblock\n", 0);
    return false;
  }

  ext2_read_block(1, sb); // Block 1 = Superblock at 1024 bytes into partition

  if (sb->s_magic != EXT2_MAGIC) {
    boot_msg("EXT2", "Invalid magic\n", 1);
    kfree(sb);
    sb = NULL;
    return false;
  }

  if (sb->s_inodes_count == 0 || sb->s_blocks_count == 0 ||
      sb->s_blocks_per_group == 0 || sb->s_inodes_per_group == 0) {
    boot_msg("EXT2", "Invalid ext2 geometry\n", 2);
    kfree(sb);
    sb = NULL;
    return false;
  }

  if (sb->s_inode_size != sizeof(ext2_inode_t)) {
    boot_msg("EXT2", "Unsupported inode size\n", 2);
    kfree(sb);
    sb = NULL;
    return false;
  }

  uint32_t block_size = 1024 << sb->s_log_block_size;
  if (block_size != BLOCK_SIZE) {
    boot_msg("EXT2", "Unsupported block size\n", 2);
    kfree(sb);
    sb = NULL;
    return false;
  }

  if (sb->s_first_data_block == 0 && sb->s_log_block_size == 0) {
    boot_msg("EXT2",
             "Detected missing first_data_block for 1KB ext2, forcing to 1\n",
             1);
    sb->s_first_data_block = 1;
  }

  if (sb->s_first_data_block >= sb->s_blocks_count) {
    boot_msg("EXT2", "Invalid first_data_block\n", 2);
    kfree(sb);
    sb = NULL;
    return false;
  }

  boot_msg("EXT2", "Mounting successful!\n", 0);
  printk("<6> EXT2: %u bytes/block | %u Inodes | first_data_block=%u "
         "blocks_per_group=%u inodes_per_group=%u\n",
         block_size, sb->s_inodes_count, sb->s_first_data_block,
         sb->s_blocks_per_group, sb->s_inodes_per_group);
  return true;
}

uint32_t ext2_alloc_inode() {
  if (!sb)
    return 0;
  if (sb->s_inodes_per_group == 0)
    return 0;

  ext2_group_desc_t gd;
  ext2_read_block(2, &gd);
  char *bitmap = kmalloc(BLOCK_SIZE);
  if (!bitmap)
    return 0;
  ext2_read_block(gd.bg_inode_bitmap, bitmap);

  for (uint32_t i = 0; i < sb->s_inodes_per_group; i++) {
    if (!(bitmap[i / 8] & (1 << (i % 8)))) {
      bitmap[i / 8] |= (1 << (i % 8));
      ext2_write_block(gd.bg_inode_bitmap, bitmap);
      kfree(bitmap);
      return i + 1;
    }
  }
  kfree(bitmap);
  return 0;
}

uint32_t ext2_alloc_block() {
  if (!sb)
    return 0;
  if (sb->s_blocks_per_group == 0)
    return 0;

  ext2_group_desc_t gd;
  ext2_read_block(2, &gd);
  char *bitmap = kmalloc(BLOCK_SIZE);
  if (!bitmap)
    return 0;
  ext2_read_block(gd.bg_block_bitmap, bitmap);

  for (uint32_t i = 0; i < sb->s_blocks_per_group; i++) {
    if (!(bitmap[i / 8] & (1 << (i % 8)))) {
      bitmap[i / 8] |= (1 << (i % 8));
      ext2_write_block(gd.bg_block_bitmap, bitmap);
      kfree(bitmap);
      return i + sb->s_first_data_block;
    }
  }
  kfree(bitmap);
  return 0;
}

/* --- Operaciones de Inodos --- */

void ext2_write_inode(uint32_t inode_no, ext2_inode_t *inode) {
  if (!sb || !inode || inode_no == 0 || inode_no > sb->s_inodes_count ||
      sb->s_inodes_per_group == 0)
    return;

  uint32_t group = (inode_no - 1) / sb->s_inodes_per_group;
  uint32_t index = (inode_no - 1) % sb->s_inodes_per_group;

  ext2_group_desc_t gds[10];
  ext2_read_block(2, gds);

  uint32_t table_block = gds[group].bg_inode_table;
  uint32_t block_offset = (index * sizeof(ext2_inode_t)) / BLOCK_SIZE;
  uint32_t byte_offset = (index * sizeof(ext2_inode_t)) % BLOCK_SIZE;

  char *buffer = kmalloc(BLOCK_SIZE);
  if (!buffer)
    return;
  ext2_read_block(table_block + block_offset, buffer);
  memcpy(buffer + byte_offset, inode, sizeof(ext2_inode_t));
  ext2_write_block(table_block + block_offset, buffer);
  kfree(buffer);
}

ext2_inode_t ext2_get_inode(uint32_t inode_no) {
  ext2_inode_t inode = {0};
  if (!sb || sb->s_inodes_per_group == 0 || inode_no == 0 ||
      inode_no > sb->s_inodes_count) {
    printk("<6> EXT2: get_inode skipped for %u inodes_per_group=%u "
           "inodes_count=%u\n",
           inode_no, sb ? sb->s_inodes_per_group : 0,
           sb ? sb->s_inodes_count : 0);
    return inode;
  }

  printk("<6> EXT2: get_inode %u on group size %u\n", inode_no,
         sb->s_inodes_per_group);
  uint32_t group = (inode_no - 1) / sb->s_inodes_per_group;
  uint32_t index = (inode_no - 1) % sb->s_inodes_per_group;
  printk("<6> EXT2: inode group=%u index=%u\n", group, index);

  ext2_group_desc_t gds[10];
  ext2_read_block(2, gds);

  uint32_t table_block = gds[group].bg_inode_table;
  uint32_t block_offset = (index * sizeof(ext2_inode_t)) / BLOCK_SIZE;
  uint32_t byte_offset = (index * sizeof(ext2_inode_t)) % BLOCK_SIZE;

  char *buffer = kmalloc(BLOCK_SIZE);
  if (!buffer)
    return inode;
  ext2_read_block(table_block + block_offset, buffer);
  memcpy(&inode, buffer + byte_offset, sizeof(ext2_inode_t));
  kfree(buffer);

  return inode;
}

static uint32_t ext2_read_file_to_buffer(ext2_inode_t *inode, char *out,
                                         uint32_t max_size) {
  if (!inode || !out)
    return 0;
  uint32_t total = 0;
  char *tmp = (char *)kmalloc(BLOCK_SIZE);
  if (!tmp)
    return 0;

  for (int i = 0; i < 12 && inode->i_block[i] && total < max_size; i++) {
    ext2_read_block(inode->i_block[i], tmp);
    uint32_t chunk = inode->i_size - total;
    if (chunk > BLOCK_SIZE)
      chunk = BLOCK_SIZE;
    if (chunk > max_size - total)
      chunk = max_size - total;
    memcpy(out + total, tmp, chunk);
    total += chunk;
    if (total >= inode->i_size)
      break;
  }

  kfree(tmp);
  return total;
}

static void ext2_import_vfs_directory(const char *path, uint32_t inode_no) {
  if (!sb || inode_no == 0 || inode_no > sb->s_inodes_count) {
    boot_msg("EXT2", "Invalid directory inode number\n", 2);
    return;
  }

  ext2_inode_t dir_inode = ext2_get_inode(inode_no);
  if (dir_inode.i_blocks == 0 || dir_inode.i_block[0] == 0) {
    boot_msg("EXT2", "Directory inode (has no blocks)\n", 0);
    return;
  }

  char *block = (char *)kmalloc(BLOCK_SIZE);
  if (!block)
    return;

  for (int i = 0; i < 12 && dir_inode.i_block[i]; i++) {
    if (dir_inode.i_block[i] >= sb->s_blocks_count) {
      boot_msg("EXT2", "Invalid block number for inode\n", 2);
      break;
    }
    ext2_read_block(dir_inode.i_block[i], block);
    uint32_t offset = 0;

    while (offset < BLOCK_SIZE) {
      ext2_dir_entry_t *entry = (ext2_dir_entry_t *)(block + offset);
      if (entry->inode == 0)
        break;
      if (entry->rec_len < 8 || entry->rec_len > BLOCK_SIZE - offset) {
        boot_msg("EXT2", "Bad dir entry rec_len", 2);
        break;
      }
      if (entry->name_len > 255 || entry->name_len > entry->rec_len - 8) {
        boot_msg("EXT2", "Bad dir entry name_len\n", 2);
        break;
      }

      char name[256];
      memcpy(name, entry->name, entry->name_len);
      name[entry->name_len] = '\0';

      if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
        if (entry->inode == 0 || entry->inode > sb->s_inodes_count) {
          boot_msg("EXT2", "Invalid child inode\n", 2);
          break;
        }

        char child_path[256];
        if (strcmp(path, "/") == 0) {
          child_path[0] = '/';
          strcpy(child_path + 1, name);
        } else {
          size_t path_len = strlen(path);
          if (path_len + 1 + entry->name_len >= sizeof(child_path)) {
            boot_msg("EXT2", "Path too long\n", 2);
            break;
          }
          memcpy(child_path, path, path_len);
          child_path[path_len] = '/';
          strcpy(child_path + path_len + 1, name);
        }

        if (entry->file_type == 2) {
          vfs_mkdir(child_path, 0755);
          ext2_import_vfs_directory(child_path, entry->inode);
        } else if (entry->file_type == 1) {
          ext2_inode_t file_inode = ext2_get_inode(entry->inode);
          if (file_inode.i_size > 0 && file_inode.i_size < 1024) {
            char *content = (char *)kmalloc(file_inode.i_size + 1);
            if (content) {
              uint32_t read = ext2_read_file_to_buffer(&file_inode, content,
                                                       file_inode.i_size);
              content[read] = '\0';
              vfs_touch(child_path, content);
            }
          } else {
            vfs_touch(child_path, "[binary file]");
          }
        }
      }

      if (entry->rec_len == 0)
        break;
      offset += entry->rec_len;
    }
  }

  kfree(block);
}

void ext2_load_vfs_root(void) {
  if (!sb) {
    boot_msg("EXT2", "Cannot load VFS root: ext2 not initialized\n", 2);
    return;
  }

  printk("<6> EXT2: ready to load root inode\n");
  vfs_mkdir("/etc", 0755);
  vfs_mkdir("/usr", 0755);
  vfs_mkdir("/bin", 0755);
  vfs_mkdir("/home", 0755);
  vfs_mkdir("/tmp", 0755);
  vfs_mkdir("/kernel", 0755);

  ext2_inode_t root_inode = ext2_get_inode(2);
  printk("<6> EXT2: root inode size=%u blocks=%u first_block=%u\n",
         root_inode.i_size, root_inode.i_blocks, root_inode.i_block[0]);
  if (root_inode.i_size == 0 || root_inode.i_blocks == 0 ||
      root_inode.i_block[0] == 0) {
    boot_msg("EXT2", "Invalid root inode\n", 2);
    return;
  }

  boot_msg("EXT2", "Importing ext2 root directory\n", 0);
  ext2_import_vfs_directory("/", 2);
  boot_msg("EXT2", "Finished ext2 root import\n", 0);
}

/* --- Comandos del FS --- */

void ext2_cat(const char *name) {
  ext2_inode_t dir_inode = ext2_get_inode(current_dir_inode);
  char *buf = (char *)kmalloc(BLOCK_SIZE);

  for (int i = 0; i < 12 && dir_inode.i_block[i]; i++) {
    ext2_read_block(dir_inode.i_block[i], buf);
    ext2_dir_entry_t *entry = (ext2_dir_entry_t *)buf;
    uint32_t offset = 0;

    while (offset < BLOCK_SIZE && entry->inode != 0) {
      if (strncmp(entry->name, name, entry->name_len) == 0) {
        ext2_inode_t file_ino = ext2_get_inode(entry->inode);
        char *data = (char *)kmalloc(BLOCK_SIZE);
        for (int j = 0; j < 12 && file_ino.i_block[j]; j++) {
          ext2_read_block(file_ino.i_block[j], data);
          printk("%s", data);
        }
        kfree(data);
        kfree(buf);
        return;
      }
      offset += entry->rec_len;
      entry = (ext2_dir_entry_t *)((uintptr_t)entry + entry->rec_len);
    }
  }
  kfree(buf);
  printk("cat: %s not found\n", name);
}

void ext2_ls() {
  ext2_inode_t inode = ext2_get_inode(current_dir_inode);
  char *buf = (char *)kmalloc(BLOCK_SIZE);

  for (int i = 0; i < 12 && inode.i_block[i]; i++) {
    ext2_read_block(inode.i_block[i], buf);
    ext2_dir_entry_t *entry = (ext2_dir_entry_t *)buf;
    uint32_t offset = 0;

    while (offset < inode.i_size && entry->inode != 0) {
      char name[256];
      memcpy(name, entry->name, entry->name_len);
      name[entry->name_len] = '\0';

      if (entry->file_type == 2) {
        printk("[%s]  ", name);
      } else {
        printk("%s  ", name);
      }

      offset += entry->rec_len;
      entry = (ext2_dir_entry_t *)((uintptr_t)entry + entry->rec_len);
    }
  }
  kfree(buf);
  printk("\n");
}

void ext2_touch(const char *name) {
  uint32_t new_ino_no = ext2_alloc_inode();
  if (new_ino_no == 0) {
    printk("touch: No free inodes\n");
    return;
  }

  ext2_inode_t new_ino = {0};
  new_ino.i_mode = 0x81ED;
  new_ino.i_size = 0;
  new_ino.i_links_count = 1;
  new_ino.i_blocks = 0;

  ext2_write_inode(new_ino_no, &new_ino);

  ext2_inode_t parent_inode = ext2_get_inode(current_dir_inode);
  char *dir_buf = (char *)kmalloc(BLOCK_SIZE);
  ext2_read_block(parent_inode.i_block[0], dir_buf);

  ext2_dir_entry_t *entry = (ext2_dir_entry_t *)dir_buf;
  uint32_t offset = 0;

  while (offset + entry->rec_len < BLOCK_SIZE) {
    offset += entry->rec_len;
    entry = (ext2_dir_entry_t *)((uintptr_t)entry + entry->rec_len);
  }

  uint32_t actual_length = 8 + entry->name_len;
  uint32_t padding = (4 - (actual_length % 4)) % 4;
  uint32_t old_rec_len = entry->rec_len;

  entry->rec_len = actual_length + padding;

  ext2_dir_entry_t *new_entry =
      (ext2_dir_entry_t *)((uintptr_t)entry + entry->rec_len);
  new_entry->inode = new_ino_no;
  new_entry->rec_len = old_rec_len - entry->rec_len;
  new_entry->name_len = strlen(name);
  new_entry->file_type = 1; // 1 = Regular File
  strcpy(new_entry->name, name);

  ext2_write_block(parent_inode.i_block[0], dir_buf);

  kfree(dir_buf);
  printk("File '%s' touched.\n", name);
}

void ext2_mkdir(const char *name) {
  uint32_t new_ino_no = ext2_alloc_inode();
  uint32_t new_blk_no = ext2_alloc_block();

  ext2_inode_t new_ino = {0};
  new_ino.i_mode = 0x41ED;
  new_ino.i_size = BLOCK_SIZE;
  new_ino.i_links_count = 2;
  new_ino.i_block[0] = new_blk_no;

  char *block_data = (char *)kmalloc(BLOCK_SIZE);
  memset(block_data, 0, BLOCK_SIZE);

  ext2_dir_entry_t *dot = (ext2_dir_entry_t *)block_data;
  dot->inode = new_ino_no;
  dot->rec_len = 12;
  dot->name_len = 1;
  dot->file_type = 2;
  strcpy(dot->name, ".");

  ext2_dir_entry_t *dotdot = (ext2_dir_entry_t *)(block_data + 12);
  dotdot->inode = current_dir_inode;
  dotdot->rec_len = BLOCK_SIZE - 12;
  dotdot->name_len = 2;
  dotdot->file_type = 2;
  strcpy(dotdot->name, "..");

  ext2_write_block(new_blk_no, block_data);
  kfree(block_data);
  printk("mkdir: %s created\n", name);
}

void ext2_rm(const char *name) {
  ext2_inode_t dir_inode = ext2_get_inode(current_dir_inode);
  char *buf = (char *)kmalloc(BLOCK_SIZE);

  for (int i = 0; i < 12 && dir_inode.i_block[i]; i++) {
    ext2_read_block(dir_inode.i_block[i], buf);
    ext2_dir_entry_t *entry = (ext2_dir_entry_t *)buf;
    ext2_dir_entry_t *prev = NULL;
    uint32_t offset = 0;

    while (offset < BLOCK_SIZE && entry->inode != 0) {
      if (strncmp(entry->name, name, entry->name_len) == 0) {
        if (prev) {
          prev->rec_len += entry->rec_len;
        } else {
          entry->inode = 0;
        }

        ext2_write_block(dir_inode.i_block[i], buf);
        kfree(buf);
        return;
      }
      offset += entry->rec_len;
      prev = entry;
      entry = (ext2_dir_entry_t *)((uintptr_t)entry + entry->rec_len);
    }
  }
  kfree(buf);
}

int ext2_cd(const char *name) {
  if (strcmp(name, "/") == 0) {
    current_dir_inode = 2;
    return 0;
  }

  ext2_inode_t dir_inode = ext2_get_inode(current_dir_inode);
  char *buf = (char *)kmalloc(BLOCK_SIZE);

  for (int i = 0; i < 12 && dir_inode.i_block[i]; i++) {
    ext2_read_block(dir_inode.i_block[i], buf);
    ext2_dir_entry_t *entry = (ext2_dir_entry_t *)buf;
    uint32_t offset = 0;

    while (offset < BLOCK_SIZE && entry->inode != 0) {

      if (strlen(name) == entry->name_len &&
          strncmp(entry->name, name, entry->name_len) == 0) {

        if (entry->file_type == 2) {
          current_dir_inode = entry->inode;
          kfree(buf);
          return 0;
        } else {
          printk("cd: %s is not a directory\n", name);
          kfree(buf);
          return -1;
        }
      }

      offset += entry->rec_len;
      entry = (ext2_dir_entry_t *)((uintptr_t)entry + entry->rec_len);
    }
  }

  kfree(buf);
  printk("cd: no such file or directory: %s\n", name);
  return -1;
}
