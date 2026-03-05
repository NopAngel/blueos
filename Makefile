#
# BlueOS / Makefile
#
# Copyright (C) 2024-2026  NopAngel <angelgabrielnieto@outlook.com>
#
#

include .config

ifeq ($(V),1)
  Q =
else
  Q = @
endif

CC      = gcc
ASM     = nasm
LD      = ld
OBJCOPY = objcopy
QEMU    = qemu-system-i386

GCC_INC = $(shell $(CC) -print-file-name=include)
CFLAGS  = -m32 -ffreestanding -fno-builtin -std=c99 -nostdlib \
          -fno-stack-protector -nostdinc -fno-pic -I. -Iinclude -I$(GCC_INC) -O2
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T arch/i386/link.ld -z noexecstack

KERNEL_BIN = kernel.bin


obj-y := arch/i386/boot.o arch/printk.o arch/i386/panic.o arch/i386/task.o lib/string.o \
         arch/i386/interrupts.o arch/i386/interrupt_entry.o arch/i386/profile.o usr/auth.o \
         arch/i386/switch.o arch/i386/bg.o drivers/multilru.o kernel/sysctl.o \
        fs/help.o drivers/tty/tty.o drivers/keyboard/keyboard.o arch/i386/syscall.o \
         drivers/scsi/scsi_core.o arch/i386/pic.o drivers/scsi/scsi_lsi.o \
         drivers/pci.o drivers/pinctrl/pinctrl.o arch/i386/idt.o arch/i386/interrupt_stubs.o \
         arch/i386/irq.o fs/vboxfs.o drivers/power/power.o \
         fs/9p.o drivers/net/mac80211.o kernel/vmcore_info.o drivers/leds/leds.o drivers/vhost/vhost_net.o \
         arch/i386/apic.o arch/i386/kvm.o kernel/hlec.o arch/i386/timer.o kernel/hpet.o kernel/shell.o \
         drivers/rtc/rtc.o drivers/battery/battery.o arch/i386/mm/memory.o arch/i386/acpi.o \
         init/kernel.o init/init_fnc.o arch/i386/intel.o arch/i386/amd.o arch/i386/interrupts-a.o \
		 drivers/isapnp/isapnp.o drivers/usb/usb.o drivers/dma/hdc_dma.o crypto/sha256.o drivers/bcma/bcma.o \
		 #kernel/userspace.o

obj-m += hello.o

obj-$(CONFIG_VFS)          += vfs.o
obj-$(CONFIG_SYSFS)        += sysfs.o
obj-y                      += fs.o    
obj-y                      += usr/editor.o 


obj-$(CONFIG_SPEAKER)      += speaker.o
obj-$(CONFIG_SOUNDBLASTER) += bls_snd.o


ifeq ($(CONFIG_MODULES),y)
    obj-y += kernel/module.o
	obj-y += drivers/bluefetch.o
endif

ifeq ($(CONFIG_MODVERSIONS),y)
    obj-y += kernel/ksyms.o
endif

ifeq ($(CONFIG_XFS),y)
	obj-y += fs/xfs.o
endif 
ifeq ($(CONFIG_JFS),y)
	obj-y += fs/jfs.o
endif


include .config

ifeq ($(CONFIG_ARCH),x86_64)
    CC      = gcc
    ASFLAGS = -f elf64
    CFLAGS  = -m64 -ffreestanding -O2 -Iinclude
    LDFLAGS = -m elf_x86_64 -T arch/i386/link.ld
    QEMU    = qemu-system-x86_64
else
    CC      = gcc
    ASFLAGS = -f elf32
    CFLAGS  = -m32 -ffreestanding -O2 -Iinclude
    LDFLAGS = -m elf_i386 -T arch/i386/link.ld
    QEMU    = qemu-system-i386
endif


RUST_LIB = rust/target/i686-unknown-linux-gnu/release/librust.a

all: $(KERNEL_BIN)

$(KERNEL_BIN): rust_module $(obj-y)
	@echo "  LD      $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $(obj-y) --whole-archive $(RUST_LIB) --no-whole-archive
	@echo "  DONE    BlueOS Kernel is ready."

%.o: %.asm
	@echo "  AS      $@"
	$(Q)$(ASM) $(ASFLAGS) $< -o $@

kernel.o: init/kernel.c
	@echo "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

init.o: init/init_fnc.c
	@echo "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.o: arch/%.c
	@echo "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

arch/%.o: arch/%.asm
	@echo "  AS      $@"
	$(Q)$(ASM) $(ASFLAGS) $< -o $@

arch/%.o: arch/%.s
	$(Q)$(ASM) -f elf32 $< -o $@

%.o: %.c
	@echo "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@


%.o: drivers/%.c
	@echo "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

keyboard.o: drivers/keyboard/keyboard.c
	@echo "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.o: fs/%.c
	@echo "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.o: usr/%.c
	@echo "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

speaker.o: sound/core/pcspeaker.c
	@echo "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

bls_snd.o: sound/blaster/blaster.s
	@echo "  AS      $@"
	$(Q)$(ASM) $(ASFLAGS) $< -o $@

# --- Rust Module ---
rust_module:
	@echo "  RUST    Building core module"
	$(Q)cd rust && RUSTFLAGS="-C relocation-model=static" cargo build --release \
		-Z build-std=core --target i686-unknown-linux-gnu

grub: all
	$(Q)mkdir -p isodir/boot/grub
	$(Q)cp kernel.bin isodir/boot/kernel.bin
	$(Q)grub-mkrescue -o BlueOS.iso isodir
# --- Helpers ---

run: all
	@mkdir -p initrd_root
#	python3 tools/mkinitrd.py
	@echo "  QEMU    Starting BlueOS with VMX emulation..."
	$(Q)$(QEMU)  \
	            -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
	            -kernel $(KERNEL_BIN) \
	            -m 256M \
				-cpu qemu32,+vmx \
				-display curses

menuconfig:
	$(Q)make -f scripts/Makefile all

clean:
	@echo "  CLEAN   Objects and binaries"
	$(Q)rm -f $() $(KERNEL_BIN) *.o *.a *.bin *.elf *.iso *.img
	$(Q)cd rust && cargo clean

.PHONY: all clean run rust_module
