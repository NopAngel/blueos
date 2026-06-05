#include <auth.h>
#include <drivers/keyboard.h>
#include <drivers/pci.h>
#include <drivers/rtc.h>
#include <drivers/serial.h>
#include <drivers/tty.h>
#include <drivers/virtio.h>
#include <drivers/virtio_net.h>
#include <drivers/vt100.h>
#include <fs/ext2.h>
#include <fs/vfs.h>
#include <fs/btrfs.h>
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

extern void print_prompt();
extern int tty_current();
int cursor_x = 0;
int cursor_y = 0;

extern void init_all(void *arch_data);
static void blueos_banner(void);

void test_bluefs() {
    printk("--- Probando BlueFS sobre Disco Real ---\n");

    // 1. Usamos la API del FS, no variables internas
    struct btrfs_disk_inode* my_inode = get_inode(1);
    my_inode->block_ptrs[0] = 10; 

    // 2. Escribimos
    const char* data = "BlueFS sobre ATA real!";
    btrfs_write_file(1, data, 22);

    // 3. Para verificar, lee usando el sistema, no mirando la memoria
    // Si tienes una función btrfs_read_file(), úsala. 
    // Si no, block_get(10) te traerá el dato desde el disco real.
    uint8_t* sector = block_get(10);
    printk("Datos leídos del disco: %s\n", sector);
}

void
k_main(unsigned int magic, void *arch_data)
{
	/* Inicialización de hardware y subsistemas */
	init_all(arch_data);

	/* Banner de inicio estilo BSD */
	blueos_banner();
	print_prompt();
 // test_bluefs();
	/* Bucle principal: idle del sistema */
	for (;;) {
		arch_idle();
	}
}

static void
blueos_banner(void)
{
	rtc_time_t now;

	get_local_time(&now);

	printk("BlueOS v%s (GENERIC) %02d/%02d/%04d-UTC-%02d:%02d\n", 
	    UTS_RELEASE, now.day, now.month, now.year, now.hour, now.minute);
	printk("Copyright (c) 2026 NopAngel. All rights reserved.\n\n");
}