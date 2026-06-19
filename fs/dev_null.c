#include <fs/vfs.h>
#include <kernel/colors.h>
#include <kernel/malloc.h>
#include <kernel/printk.h>
#include <lib/string.h>

static vfs_node_t *vfs_root = NULL;
static vfs_node_t *vfs_current = NULL;

vfs_node_t *vfs_lookup(const char *path) {
  if (!path)
    return NULL;
  if (strcmp(path, "/") == 0)
    return vfs_root;

  vfs_node_t *curr = (path[0] == '/') ? vfs_root : vfs_current;

  char tmp_path[256];
  strncpy(tmp_path, path, 255);

  char *part = strtok(tmp_path, "/");
  while (part != NULL) {
    if (curr->ops && curr->ops->finddir) {
      curr = curr->ops->finddir(curr, part);
      if (!curr)
        return NULL;
    } else {
      return NULL;
    }
    part = strtok(NULL, "/");
  }
  return curr;
}

void vfs_ls(const char *path) {
  vfs_node_t *dir =
      (path && path[0] != '\0') ? vfs_lookup(path) : vfs_get_current();

  if (!dir) {
    printk("ls: %s: No such directory\n", path);
    return;
  }

  if (dir->type != VFS_TYPE_DIR) {
    printk("ls: %s: Not a directory\n", path);
    return;
  }

  struct vfs_dirent entry;

  for (uint32_t i = 0;; i++) {
    if (dir->ops && dir->ops->readdir) {
      if (dir->ops->readdir(dir, i, &entry) != 0) {
        break;
      }

      if (entry.type == VFS_TYPE_DIR)
        printk("%s/  ", entry.name);
      else
        printk("%s  ", entry.name);
    } else {
      break;
    }
  }
  printk("\n");
}
