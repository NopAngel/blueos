#
# BlueOS / Makefile
#
# Copyright (C) 2024-2026  NopAngel <angelgabrielnieto@outlook.com>
#
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
         arch/switch.o arch/bg.o auth.o drivers/multilru.o kernel/sysctl.o \
		 arch/mm/memory.o fs/help.o drivers/tty.o  \
		 drivers/scsi/scsi_core.o arch/pic.o drivers/scsi/scsi_lsi.o \
		 drivers/pci.o drivers/pinctrl/pinctrl.o arch/idt.o arch/interrupt_stubs.o \
		 arch/irq.o drivers/pictrl.o fs/vboxfs.o arch/interrupt_entry.o drivers/power/power.o \
		 fs/9p.o drivers/net/mac80211.o kernel/vmcore_info.o drivers/leds.o drivers/vhost_net.o \
		 arch/apic.o arch/kvm.o 

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
    LDFLAGS = -m elf_x86_64 -T link.ld
    QEMU    = qemu-system-x86_64
else
    CC      = gcc
    ASFLAGS = -f elf32
    CFLAGS  = -m32 -ffreestanding -O2 -Iinclude
    LDFLAGS = -m elf_i386 -T link.ld
    QEMU    = qemu-system-i386
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
	$(Q)$(QEMU) -cpu pentium3,+vmx \
	            -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
	            -kernel $(KERNEL_BIN) \
	            -m 256M 

#run: all
#	@echo "  QEMU    $(KERNEL_BIN)"
#	$(Q)$(QEMU) -d int -no-reboot -no-shutdown -cpu qemu32,+vmx -kernel $(KERNEL_BIN) \
#		-m 256M 




# --- Limine Config (Binary Branch) ---
LIMINE_GIT_URL = https://github.com/limine-bootloader/limine.git
LIMINE_BRANCH  = v8.x-binary
ISO_DIR        = isodir
INITRD_DIR     = initrd_root

# --- Build Rules ---

all: $(KERNEL_BIN)

# 1. Descarga Limine (Solo los binarios de la rama v7.x)
limine-setup:
	@if [ ! -d "limine" ]; then \
		echo "  GIT      Cloning Limine..."; \
		git clone https://github.com/limine-bootloader/limine --branch=v8.x-binary --depth=1; \
		$(MAKE) -C limine; \
	fi

# 2. Empaquetar el Initrd en TAR (Limine ama los .tar)
initrd:
	@echo "  TAR     Creating initrd.tar..."
	@mkdir -p $(INITRD_DIR)
	@# Ejemplo: Meter el hello.txt si existe
	@if [ -f "hello.txt" ]; then cp hello.txt $(INITRD_DIR)/; fi
	$(Q)tar -cf initrd.tar -C $(INITRD_DIR) .

#limine: $(KERNEL_BIN) limine limine.cfg
#	@echo "  ISO      Building blueos.iso..."
#	@mkdir -p iso_root
#	@cp $(KERNEL_BIN) iso_root/kernel.bin
#	@cp limine.cfg iso_root/
#	@cp limine/limine-bios.sys limine/limine-bios-cd.bin \
#	    limine/limine-uefi-cd.bin iso_root/
#	@xorriso -as mkisofs -b limine-bios-cd.bin \
#	    -no-emul-boot -boot-load-size 4 -boot-info-table \
#	    --protective-msdos-label \
#	    -partition_offset 16 \
#	    iso_root -o blueos.iso > /dev/null 2>&1
#	@./limine/limine bios-install blueos.iso
#	@rm -rf iso_root


limine: $(KERNEL_BIN) limine-setup initrd limine.cfg
	@echo "  ISO      Building blueos.iso..."
	@mkdir -p iso_root
	@cp $(KERNEL_BIN) iso_root/kernel.bin
	@cp limine.cfg iso_root/
	@# --- EL FIX AQUÍ: Copiar el initrd a la ISO ---
	@cp initrd.tar iso_root/
	@# ---------------------------------------------
	@cp limine/limine-bios.sys limine/limine-bios-cd.bin \
	    limine/limine-uefi-cd.bin iso_root/
	@xorriso -as mkisofs -b limine-bios-cd.bin \
	    -no-emul-boot -boot-load-size 4 -boot-info-table \
	    --protective-msdos-label \
	    -partition_offset 16 \
	    iso_root -o blueos.iso > /dev/null 2>&1
	@./limine/limine bios-install blueos.iso

run-limine:
	@echo "  QEMU    Booting BlueOS.iso..."
	$(Q)$(QEMU) -cdrom  BlueOS.iso -m 256M -serial stdio -d int







menuconfig:
	$(Q)make -f scripts/Makefile all

clean:
	@echo "  CLEAN   Objects and binaries"
	$(Q)rm -f $(obj-y) $(KERNEL_BIN) *.o *.a *.bin *.elf *.iso *.img
	$(Q)cd rust && cargo clean

clean_limine:
	@echo "  CLEAN   Limine build artifacts"
	$(Q)rm -rf limine iso_root initrd.tar





.PHONY: all clean run rust_module