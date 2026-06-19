#
# BlueOS Universal Makefile 
#     (B U M)
#

-include .config

SHELL := /bin/sh
MAKEFLAGS += --no-print-directory

BUILD_DIR ?= build
GEN_DIR   ?= include/generated
DEPFLAGS  ?= -MMD -MP

ARCH ?= x86
V    ?= 0
Q     = $(if $(filter 1,$(V)),,@)


OBJCOPY := objcopy
NM      := nm
SIZE    := size
FORMAT  := clang-format
DOXYGEN := doxygen

ifeq ($(ARCH),x86)
    CC         = x86_64-linux-gnu-gcc
    ASM        = nasm
    LD         = ld
    QEMU       = qemu-system-i386
    GCC_INC    = $(shell $(CC) -print-file-name=include 2>/dev/null)
    CFLAGS     = -m32 -march=i386 -mno-sse -mno-sse2 -mno-mmx -ffreestanding \
                 -fno-builtin -std=gnu99 -nostdlib -fno-stack-protector \
                 -nostdinc -fno-pic -fno-pie -I. -Iinclude -I$(GCC_INC) -O2 \
                 -g -Dx86 -D__x86__ $(EXTRA_CFLAGS)
    ASFLAGS    = -f elf32
    LDFLAGS    = -m elf_i386 -T arch/x86/link.ld -z noexecstack $(EXTRA_LDFLAGS)
    KERNEL_BIN = $(BUILD_DIR)/blueos.elf
    QEMU_FLAGS = -kernel $(KERNEL_BIN) -serial stdio
else ifeq ($(ARCH),riscv)
    CC         = riscv64-unknown-elf-gcc
    ASM        = riscv64-unknown-elf-gcc
    LD         = riscv64-unknown-elf-ld
    QEMU       = qemu-system-riscv32
    NM         = riscv64-unknown-elf-nm
    SIZE       = riscv64-unknown-elf-size
    CFLAGS     = -march=rv32i_zicsr -mabi=ilp32 -ffreestanding -fno-builtin \
                 -std=gnu99 -nostdlib -fno-stack-protector -fno-pic -fno-pie \
                 -O0 -g -DRISCV -I. -Iinclude $(EXTRA_CFLAGS)
    ASFLAGS    = -march=rv32i_zicsr -mabi=ilp32 -c
    LDFLAGS    = -m elf32lriscv -T arch/riscv/link.ld $(EXTRA_LDFLAGS)
    KERNEL_BIN = $(BUILD_DIR)/blueos.elf
    QEMU_FLAGS = -machine virt -bios default -kernel $(KERNEL_BIN) -serial stdio
endif

SUBDIRS_NEED := arch/common kernel kernel/mm drivers/ata drivers/disk drivers/amba \
                 drivers/connector drivers/i2c drivers/thermal drivers/soc drivers/usb \
                 drivers/pci fs/vfs fs/ramfs fs/ext2 init drivers/tty kernel/power

SUBDIRS      := arch/$(ARCH) $(SUBDIRS_NEED) drivers/leds \
                 drivers/vhost drivers/rtc drivers/battery drivers/net \
                 drivers/isapnp drivers/video drivers/bcma \
                 drivers/pnp drivers/hyperv drivers/core \
                 drivers/cdrom drivers/gpio drivers/input/joystick/ps3 \
                 drivers/input/mouse/8042 drivers/input/keyboard/s390 \
                 drivers/input/keyboard drivers/scsi drivers/pinctrl \
                 lib crypto usr net fs/btrfs fs/tmpfs drivers/video/vt \
				 drivers/extcon drivers/eisa fs/xfs fs/initramfs

obj-y        :=

EXISTING_MAKEFILES := $(wildcard $(patsubst %,%/Makefile,$(SUBDIRS)))
include $(EXISTING_MAKEFILES)

KERNEL_OBJS := $(patsubst %,$(BUILD_DIR)/%,$(obj-y))

export CC CFLAGS ASFLAGS ARCH LD Q

.PHONY: all clean run debug iso prepare check_tools format help test doc

all: check_tools prepare $(KERNEL_BIN)

check_tools:
	$(Q)command -v $(CC) >/dev/null 2>&1 || (echo -e "ERROR: GCC was not found. $(CC). Please install it or adjust your environment."; exit 1)
	$(Q)command -v $(LD) >/dev/null 2>&1 || (echo -e "ERROR: The linker was not found. $(LD)."; exit 1)
ifeq ($(ARCH),x86)
	$(Q)command -v $(ASM) >/dev/null 2>&1 || (echo -e "ERROR: 'nasm' was not found. It is required for x86 architecture."; exit 1)
endif

$(KERNEL_BIN): $(KERNEL_OBJS)
	@echo -e "  LD       $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)
    
$(BUILD_DIR)/%.o: %.c
	@echo -e "  CC        $<"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@echo -e "  AS        $<"
	@mkdir -p $(dir $@)
ifeq ($(ARCH),x86)
	$(Q)$(ASM) $(ASFLAGS) $< -o $@
else
	$(Q)$(CC) $(CFLAGS) -c $< -o $@
endif

$(BUILD_DIR)/%.o: %.asm
	@echo -e "  ASM       $<"
	@mkdir -p $(dir $@)
	$(Q)$(ASM) $(ASFLAGS) $< -o $@

prepare:
	$(Q)mkdir -p $(BUILD_DIR) $(GEN_DIR)
	@echo -e "  GEN       $(GEN_DIR)/utsrelease.h"
	$(Q)echo '#define BLUEOS_VERSION "0.1.0-alpha"' > $(GEN_DIR)/utsrelease.h
	@echo -e "  GEN       $(GEN_DIR)/version.h"
	$(Q)echo '#define UTS_VERSION "#1 SMP PREEMPT '$(shell date +"%Y-%m-%d %H:%M:%S")'"' > $(GEN_DIR)/version.h

format:
	@echo -e "  FORMAT    Formatting BlueOS code with clang-format..."
	$(Q)find . -maxdepth 3 -name "*.c" -o -name "*.h" | xargs $(FORMAT) -i --style=file 2>/dev/null || echo "WARNING: Install 'clang-format' to autoformat the code."

run: all
	@echo -e "  QEMU      Running BlueOS ($(ARCH))..."
	$(Q)$(QEMU) $(QEMU_FLAGS) -d int

debug: all
	@echo -e "  GDB       Starting QEMU in debug mode on port 1234..."
	@echo -e "  GDB       Open another terminal and run: gdb $(KERNEL_BIN)"
	$(Q)$(QEMU) $(QEMU_FLAGS) -s -S

iso: all
ifeq ($(ARCH),x86)
	@echo -e "  ISO       Creating bootable ISO image for BlueOS..."
	$(Q)mkdir -p $(BUILD_DIR)/isofiles/boot/grub
	$(Q)cp $(KERNEL_BIN) $(BUILD_DIR)/isofiles/boot/blueos.elf
	$(Q)echo 'set default=0' > $(BUILD_DIR)/isofiles/boot/grub/grub.cfg
	$(Q)echo 'set timeout=0' >> $(BUILD_DIR)/isofiles/boot/grub/grub.cfg
	$(Q)echo 'menuentry "BlueOS" {' >> $(BUILD_DIR)/isofiles/boot/grub/grub.cfg
	$(Q)echo '  multiboot /boot/blueos.elf' >> $(BUILD_DIR)/isofiles/boot/grub/grub.cfg
	$(Q)echo '  boot' >> $(BUILD_DIR)/isofiles/boot/grub/grub.cfg
	$(Q)echo '}' >> $(BUILD_DIR)/isofiles/boot/grub/grub.cfg
	$(Q)grub-mkrescue -o blueos.iso $(BUILD_DIR)/isofiles 2>/dev/null
	@echo -e "  ISO       Done."
else
	@echo "The 'iso' target is only supported on x86 architecture for now."
endif

test:
	@echo -e "  TEST      Compiling and running host environment tests..."
	$(Q)mkdir -p $(BUILD_DIR)/tests
	$(Q)gcc -I. -Iinclude tests/test_main.c -o $(BUILD_DIR)/tests/runner 2>/dev/null || echo "WARNING: Create tests/test_main.c to run logic tests."
	$(Q)[ -f $(BUILD_DIR)/tests/runner ] && $(BUILD_DIR)/tests/runner || true


doc:
	@echo -e "  DOC       Generating documentation with Doxygen..."
	$(Q)command -v $(DOXYGEN) >/dev/null 2>&1 || (echo "WARNING: Install 'doxygen' to generate documentation."; exit 1)
	$(Q)(doxygen -g $(BUILD_DIR)/Doxyfile && echo "OUTPUT_DIRECTORY = $(BUILD_DIR)/doc" >> $(BUILD_DIR)/Doxyfile && echo "INPUT = kernel drivers fs lib" >> $(BUILD_DIR)/Doxyfile) 2>/dev/null
	$(Q)$(DOXYGEN) $(BUILD_DIR)/Doxyfile >/dev/null 2>&1
	@echo -e "  DOC       $(BUILD_DIR)/doc/html/index.html"

clean:
	@echo -e "  CLEAN     $(BUILD_DIR)"
	$(Q)rm -rf $(BUILD_DIR) blueos.iso initrd.tar
	@echo "  CLEAN     $(GEN_DIR)"
	$(Q)rm -rf $(GEN_DIR)

help:
	@echo -e "Use: make [meta] [options]"
	@echo -e ""
	@echo -e " OPTIONS: "
	@echo -e "  all          - Compiles the entire kernel (Default)"
	@echo -e "  clean        - Remove the build directory and generated files"
	@echo -e "  run          - Compile and launch the operating system in QEMU"
	@echo -e "  debug        - QEMU launches frozen, waiting for GDB connection on port 1234"
	@echo -e "  iso          - BlueOS ISO GRUB (only x86)"
	@echo -e "  format       - Applies 'clang-format' recursively to all source code"
	@echo -e "  test         - Runs unit tests for algorithm logic on the host"
	@echo -e "  doc          - Automatically generates interactive technical documentation in HTML"
	@echo -e ""
	@echo -e "Configuration Options:"
	@echo -e "  ARCH=x86     - Sets the target for 32-bit Intel/AMD systems (Default)"
	@echo -e "  ARCH=riscv   - Sets the target for RISC-V 32-bit architecture emulation"
	@echo -e "  V=1          - Verbose mode. Shows the raw command lines of GCC/LD"
	@echo -e ""

APPS := touch echo ls cd main cat reboot

USER_APPS_BIN := $(patsubst %, initrd/bin/%, $(APPS))

USER_OBJS     := $(patsubst %, build/user/%.o, $(APPS)) build/user/libuser.o

CFLAGS_USER = -m32 -march=i386 -ffreestanding -fno-builtin -nostdlib \
              -fno-stack-protector -nostdinc -fno-pic -fno-pie \
              -fno-asynchronous-unwind-tables -Iinclude

initrd.tar: $(USER_APPS_BIN)
	@echo "  TAR       Empaquetando initrd..."
	@mkdir -p initrd/bin initrd/etc
	@echo "README.TXT" > initrd/README.TXT
	@echo "Welcome to BlueOS! @Copyright 2026 GPL-3.0" > initrd/etc/motd
	@rm -f initrd.tar  
	@cd initrd && tar --format=ustar -cvf ../initrd.tar bin etc README.TXT

initrd/bin/%: build/user/%.o build/user/libuser.o
	@echo "  LD        $@"
	@mkdir -p initrd/bin        
	$(Q)$(LD) -m elf_i386 -T sbin/user.ld build/user/$*.o build/user/libuser.o -o build/user/$*.elf
	$(Q)$(OBJCOPY) -O binary build/user/$*.elf $@

build/user/libuser.o: sbin/libuser.c
	@mkdir -p build/user
	@echo "  CC (USR)  $<"
	$(Q)$(CC) $(CFLAGS_USER) -c $< -o $@

build/user/%.o: sbin/%.c
	@mkdir -p build/user
	@echo "  CC (USR)  $<"
	$(Q)$(CC) $(CFLAGS_USER) -c $< -o $@
