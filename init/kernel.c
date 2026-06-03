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

void _blueos_banner() {
  rtc_time_t now;

  get_local_time(&now);

  printk("BlueOS v%s \033[33m(GENERIC)\033[0m %02d/%02d/%04d-UTC-%02d:%02d\n\n", UTS_RELEASE,
         now.day, now.month, now.year, now.hour, now.minute);
  printk(" -     ISSUE  : https://github.com/NopAngel/blueos/issues\n");
  printk(" -     SOURCE : https://github.com/NopAngel/blueos/\n");
  printk(" -     WEBSITE: https://bluekernel.vercel.app/\n");
}

void k_main(unsigned int magic, void *arch_data) {
  init_all(arch_data);

  _blueos_banner();
  print_prompt();
  while (1) {
    arch_idle();
  }
}
