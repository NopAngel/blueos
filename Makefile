#
# BlueOS Universal Makefile
#     (B U M)
#

-include .config

SHELL := /bin/sh
MAKEFLAGS += --no-print-directory

BUILD_DIR ?= build
GEN_DIR ?= include/generated
DEPFLAGS ?= -MMD -MP

# --- Basic Configuration ---
ARCH ?= x86
V ?= 0
Q = $(if $(filter 1,$(V)),,@)

ifeq ($(ARCH),x86)
    CC      = x86_64-linux-gnu-gcc
    ASM     = nasm
    LD      = ld
    QEMU    = qemu-system-x86_64
    GCC_INC = $(shell $(CC) -print-file-name=include)
    CFLAGS  = -m32 -march=i386 -mno-sse -mno-sse2 -mno-mmx -ffreestanding -fno-builtin -std=gnu99 -nostdlib \
              -fno-stack-protector -nostdinc -fno-pic -I. -Iinclude -I$(GCC_INC) -O2 \
              -Dx86 -D__x86__
    ASFLAGS = -f elf32
    LDFLAGS = -m elf_i386 -T arch/x86/link.ld -z noexecstack
    KERNEL_BIN = blueos.elf
else ifeq ($(ARCH),riscv)
    CC      = riscv64-unknown-elf-gcc
    LD      = ld
    QEMU    = qemu-system-riscv32
    CFLAGS  = -march=rv32i_zicsr -mabi=ilp32 -ffreestanding -fno-builtin -std=gnu99 -nostdlib -fno-stack-protector -fno-pic -O0 -g -DRISCV -Iinclude
    LDFLAGS = -m elf32lriscv -T arch/riscv/link.ld
    KERNEL_BIN = blueos.elf
endif

SUBDIRS := arch/$(ARCH) arch/common kernel kernel/mm drivers/leds drivers/vhost drivers/rtc drivers/battery drivers/net drivers/isapnp \
           drivers/video drivers/bcma drivers/ata drivers/disk drivers/amba drivers/connector drivers/i2c drivers/thermal drivers/soc \
           drivers/pnp drivers/hyperv drivers/core drivers/cdrom drivers/tty drivers/gpio drivers/usb drivers/input/joystick/ps3 drivers/input/mouse/8042 \
           drivers/input/keyboard/8042 drivers/input/keyboard drivers/scsi drivers/pinctrl drivers/pci lib crypto usr init net \
           fs/ext2 fs/btrfs fs/ramfs fs/tmpfs fs/vfs
obj-y :=

EXISTING_MAKEFILES := $(wildcard $(patsubst %,%/Makefile,$(SUBDIRS)))
include $(EXISTING_MAKEFILES)

export CC CFLAGS ASFLAGS ARCH LD Q

.PHONY: all clean run iso version_h prepare

all: prepare version_h $(KERNEL_BIN)

$(KERNEL_BIN): $(obj-y)
	@echo "  LD        $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $(obj-y)

%.o: %.c
	@echo "  CC        $<"
	$(Q)$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

%.o: %.S
	@echo "  AS (NASM) $<"
	$(Q)$(ASM) $(ASFLAGS) $< -o $@

%.elf: %.c
	@echo "  CC_USER   $<"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@.o
	$(Q)$(LD) $(LDFLAGS) -o $@ $@.o

%.o: %.asm
	@echo "  AS (NASM) $<"
	$(Q)$(ASM) $(ASFLAGS) $< -o $@
prepare:
	@mkdir -p build include/generated
	@echo "#define BLUEOS_VERSION \"0.1.0-alpha\"" > include/generated/utsrelease.h

version_h:
	@echo "  UPD       include/version.h"
	$(Q)sed -i 's/#define UTS_VERSION.*/#define UTS_VERSION    "#1 SMP PREEMPT '"$$(date +'%Y-%m-%d %H:%M:%S')"'"/' include/version.h 2>/dev/null || true

clean:
	@echo "  CLEAN      Objects"
	$(Q)rm -f $(KERNEL_BIN) *.elf *.iso
	$(Q)find . -name "*.o" -type f -delete
	$(Q)find . -name "*.d" -type f -delete