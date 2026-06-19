#include <kernel/colors.h>
#include <kernel/printk.h>
#include <lib/string.h>

struct tar_header {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag;
  char linkname[100];
  char magic[6];
  char version[2];
};

static int octal_to_int(const char *str) {
  int val = 0;
  while (*str >= '0' && *str <= '7') {
    val = (val << 3) + (*str - '0');
    str++;
  }
  return val;
}

void *initrd_find_file(void *tar_address, const char *name,
                       unsigned int *out_size) {
  struct tar_header *header = (struct tar_header *)tar_address;

  while (header->name[0] != '\0') {
    int size = octal_to_int(header->size);

    if (strcmp(header->name, name) == 0) {
      if (out_size)
        *out_size = size;
      return (void *)((char *)header + 512);
    }

    int offset = 512 + ((size + 511) & ~511);
    header = (struct tar_header *)((char *)header + offset);

    if ((unsigned int)header >= 0x10000000)
      break;
  }
  return NULL;
}