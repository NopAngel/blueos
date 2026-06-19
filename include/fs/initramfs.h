#ifndef _BLUEOS_INITRAMFS_H_
#define _BLUEOS_INITRAMFS_H_

#include <stddef.h>
#include <stdint.h>


void initramfs_parse(uintptr_t ramdisk_start, uintptr_t ramdisk_end);

#endif /* _BLUEOS_INITRAMFS_H_ */