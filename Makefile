#
# BlueOS / Makefile
#
# Copyright (C) 2024-2026  NopAngel <angelgabrielnieto@outlook.com>
#
# Inspired by the Linux Kernel Kbuild system and Vim's Makefile.
#

include .config

# --- Verbosity Control ---
# Use 'make V=1' to see full commands
ifeq ($(V),1)
  Q =
else
  Q = @
endif

# --- Toolchain ---
CC      = gcc
ASM     = nasm
LD      = ld
OBJCOPY = objcopy
QEMU    = qemu-system-i386

# --- Compilation Flags ---
GCC_INC = $(shell $(CC) -print-file-name=include)
CFLAGS  = -m32 -ffreestanding -fno-builtin -std=c99 -nostdlib \
          -fno-stack-protector -nostdinc -fno-pic -I. -Iinclude -I$(GCC_INC) -O2
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T link.ld -z noexecstack

KERNEL_BIN = kernel.bin


obj-y := boot.o init/kernel.o printk.o panic.o task.o init.o lib/string.o \
         arch/interrupts.o arch/interrupts-a.o syscall.o profile.o notifier.o \
         arch/switch.o bg.o auth.o drivers/multilru.o kernel/sysctl.o \
		 arch/mm/memory.o fs/help.o

obj-m += hello.o

# Filesystem & Shell
obj-$(CONFIG_VFS)          += vfs.o
obj-$(CONFIG_SYSFS)        += sysfs.o
obj-y                      += fs.o    
obj-y                      += usr/editor.o 

# Drivers & Audio
obj-$(CONFIG_KEYBOARD)     += keyboard.o
obj-$(CONFIG_SPEAKER)      += speaker.o
obj-$(CONFIG_SOUNDBLASTER) += bls_snd.o


ifeq ($(CONFIG_MODULES),y)
    obj-y += kernel/module.o
	obj-y += drivers/bluefetch.o
endif

ifeq ($(CONFIG_MODVERSIONS),y)
    obj-y += kernel/ksyms.o
endif

# Rust support
RUST_LIB = rust/target/i686-unknown-linux-gnu/release/librust.a

# --- Build Rules ---

all: $(KERNEL_BIN)

$(KERNEL_BIN): rust_module $(obj-y)
	@echo "  LD      $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $(obj-y) --whole-archive $(RUST_LIB) --no-whole-archive
	@echo "  DONE    BlueOS Kernel is ready."

# Pattern rule for Assembly files
%.o: %.asm
	@echo "  AS      $@"
	$(Q)$(ASM) $(ASFLAGS) $< -o $@

# Specific rules for directories (Linux uses Kbuild, we use pattern mapping)
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

# --- Helpers ---

run: all
	@echo "  QEMU    $(KERNEL_BIN)"
	$(Q)$(QEMU) -no-reboot -no-shutdown -kernel $(KERNEL_BIN)

clean:
	@echo "  CLEAN   Objects and binaries"
	$(Q)rm -f $(obj-y) $(KERNEL_BIN) *.o *.a *.bin *.elf
	$(Q)cd rust && cargo clean

.PHONY: all clean run rust_module