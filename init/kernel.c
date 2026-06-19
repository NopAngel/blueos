#include <auth.h>
#include <drivers/keyboard.h>
#include <drivers/pci.h>
#include <drivers/rtc.h>
#include <drivers/serial.h>
#include <drivers/tty.h>
#include <drivers/virtio.h>
#include <drivers/virtio_net.h>
#include <drivers/vt100.h>
#include <fs/btrfs.h>
#include <fs/ext2.h>
#include <fs/vfs.h>
#include <fs/xfs.h>
#include <kernel/arch.h>
#include <kernel/colors.h>
#include <kernel/malloc.h>
#include <kernel/panic.h>
#include <kernel/printk.h>
#include <kernel/timer.h>
#include <lib/string.h>
#include <mm/memory.h>
#include <multiboot.h>
#include <version.h>

extern void print_prompt(); // Tu función para pintar la línea de comandos
extern int tty_current();
int cursor_x = 0;
int cursor_y = 0;

extern struct vfs_node *g_root_node;
extern vfs_ops_t xfs_ops;
extern void initramfs_parse(uintptr_t ramdisk_start, uintptr_t ramdisk_end);

extern void xfs_init(void);
extern void init_all(unsigned int magic, void *arch_data);
static void blueos_banner(void);

void k_main(unsigned int magic, void *arch_data) {
    init_all(magic, arch_data);

    
    blueos_banner();
    print_prompt();


    for (;;) { 
        arch_idle(); 
    }
}

static void blueos_banner(void) {
  rtc_time_t now;
  get_local_time(&now);

  printk("BlueOS v%s (GENERIC) %02d/%02d/%04d-UTC-%02d:%02d\n", UTS_RELEASE,
         now.day, now.month, now.year, now.hour, now.minute);
  printk("Copyright (c) 2026 NopAngel. All rights reserved.\n");
}