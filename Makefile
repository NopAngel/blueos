#
# BlueOS Universal Makefile
# Core build system for i386 and RISC-V
#

-include .config

# --- Basic Configuration ---
ARCH ?= i386
V ?= 0
ifeq ($(V),1)
  Q =
else
  Q = @
endif

# --- Toolchain & Flags Setup ---
ifeq ($(ARCH),i386)
    # i386 Specific
    CC      = gcc
    ASM     = nasm
    LD      = ld
    QEMU    = qemu-system-i386
    GDB     = gdb

    GCC_INC = $(shell $(CC) -print-file-name=include)
    CFLAGS  = -m32 -ffreestanding -fno-builtin -std=gnu99 -nostdlib \
              -fno-stack-protector -nostdinc -fno-pic -I. -Iinclude -I$(GCC_INC) -O2 \
              -DI386 -D__i386__
    ASFLAGS = -f elf32
    LDFLAGS = -m elf_i386 -T arch/i386/link.ld -z noexecstack

    KERNEL_BIN = kernel.elf
    KERNEL_ISO = blueos.iso
    RUST_TARGET = i686-unknown-linux-gnu
    RUST_LIB = rust/target/$(RUST_TARGET)/release/librust.a

    # Objects for i386
    obj-y := arch/i386/boot.o arch/i386/loader.o arch/printk.o arch/i386/panic.o arch/i386/task.o lib/string.o \
             arch/i386/interrupts.o arch/i386/interrupt_entry.o arch/i386/profile.o usr/auth.o \
             arch/i386/switch.o arch/i386/bg.o drivers/multilru.o \
             fs/help.o drivers/tty/tty.o drivers/keyboard/8042/keyboard.o arch/i386/syscall.o \
             drivers/scsi/scsi_core.o arch/pic.o drivers/scsi/scsi_lsi.o \
             drivers/pci/pci.o drivers/pinctrl/pinctrl.o arch/i386/idt.o \
             arch/i386/irq.o kernel/sysctl.o fs/vboxfs/vboxfs.o drivers/power/power.o arch/i386/keyboard_io.o \
             fs/9p/9p.o kernel/vmcore_info.o drivers/leds/leds.o drivers/vhost/vhost_net.o \
             arch/i386/apic.o arch/i386/kvm.o kernel/hlec.o arch/i386/timer.o kernel/hpet.o arch/i386/shell.o \
             drivers/rtc/rtc.o drivers/battery/battery.o arch/i386/mm/memory.o arch/i386/acpi.o \
             arch/i386/kernel.o arch/i386/init_fnc.o arch/i386/intel.o arch/i386/amd.o arch/i386/interrupts-a.o \
             drivers/isapnp/isapnp.o arch/i386/isr.o drivers/usb/usb.o drivers/dma/hdc_dma.o crypto/sha256.o drivers/bcma/bcma.o \
             kernel/module.o drivers/bluefetch.o kernel/ksyms.o fs/vfs/vfs.o \
             arch/i386/gdt.o arch/i386/gdt-a.o arch/i386/virt.o \
             fs/ramfs/fs.o sound/sdw/s.o drivers/net/rtl8139.o lib/network.o \
             fs/fat32/fat32.o drivers/ata/ata.o init/hyper.o arch/i386/cmdline.o drivers/amba/amba_pl011.o \
             fs/9p/vfs_9p.o drivers/virtio/virtio_9p.o fs/ext2/ext2.o arch/i386/commands.o  \
             drivers/connector/connector.o drivers/i2c/i2c.o drivers/thermal/lm75.o drivers/soc/soc_intel.o \
             drivers/pnp/pnp.o drivers/core/live_config.o arch/i386/pm.o fs/fat16/fat16.o kernel/mm/malloc.o \
             arch/i386/hal.o fs/dev_null.o

else ifeq ($(ARCH),riscv)
    # RISC-V Specific
    CC      = riscv64-unknown-elf-gcc
    AS      = riscv64-unknown-elf-as
    LD      = riscv64-unknown-elf-ld
    QEMU    = qemu-system-riscv32

    GCC_INC = $(shell $(CC) -print-file-name=include)
    INCLUDES = -Iinclude -Iinclude/kernel -Iinclude/arch/riscv -Iinclude/drivers -I$(GCC_INC) -I.
    CFLAGS  = -march=rv32i_zicsr -mabi=ilp32 -ffreestanding -fno-builtin -std=gnu99 -nostdlib \
              -fno-stack-protector -nostdinc -fno-pic -O0 -g -DRISCV $(INCLUDES) \
              -march=rv32imafdc_h_zicsr_zifencei_zbkb_zbkc_zbkx_zknd_zkne_zknh
    ASFLAGS = -march=rv32i_zicsr -mabi=ilp32 -g
    LDFLAGS = -m elf32lriscv -T arch/riscv/link.ld

    KERNEL_BIN = build/blueos_riscv.elf


    # Objects for RISC-V
    obj-y := arch/riscv/boot.o arch/riscv/kernel.o arch/printk.o arch/riscv/acpi.o \
             arch/riscv/plic.o arch/riscv/trap.o arch/riscv/trap_entry.o arch/riscv/trap_handler.o \
             arch/riscv/interrupts.o arch/riscv/panic.o arch/riscv/commands.o arch/riscv/keyboard_io.o \
             arch/riscv/shell.o arch/riscv/profile.o arch/riscv/kvm.o arch/riscv/mm/memory.o \
             arch/riscv/vendor_h.o arch/riscv/intel_h.o arch/riscv/init_fnc.o arch/riscv/task.o \
             arch/riscv/switch.o drivers/multilru.o drivers/vhost/vhost_net.o \
             drivers/battery/battery.o drivers/bcma/bcma.o drivers/connector/connector.o \
             drivers/pnp/pnp.o drivers/core/live_config.o fs/ramfs/fs.o fs/eventfs.o \
             lib/string.o crypto/sha256.o usr/auth.o sound/snd.o kernel/sched.o \
             arch/riscv/timer.o kernel/mm/vmm.o lib/list.o \
             crypto/aes.o crypto/aes_riscv_glue.o crypto/sm3_riscv_glue.o arch/riscv/fpu.o \
             kernel/power/hibernate.o arch/riscv/hibernate_asm.o arch/riscv/intr.o \
             drivers/power/power.o kernel/jump_label.o arch/riscv/kvm/nmu.o \
             kernel/net/net.o kernel/bpf/jit_rv64.o kernel/mm/fault.o kernel/process.o \
             kernel/mm/demand_paging.o kernel/mm/pmm.o kernel/mm/ptdump.o drivers/xen/xen.o \
             drivers/watchdog/wdt.o drivers/zorro/zorro.o drivers/android/binder.o \
             drivers/android/ashmem.o drivers/mailbox/mbox.o drivers/ps3/ps3_ds3.o \
             drivers/tc/tc.o drivers/cxl/cxl_m.o drivers/ptp/ptp.o drivers/pps/pps.o \
             drivers/firmware/firmware.o drivers/w1/w1.o drivers/hal/gpio.o \
             drivers/hal/timer.o drivers/dax/dax.o drivers/edac/edac.o drivers/uio/uio.o \
             drivers/macintosh/adb.o drivers/hx/haptics.o drivers/i2c/i2c.o fs/fat16/fat16.o \
             kernel/mm/malloc.o arch/riscv/hal.o
endif

# --- Global Logic ---
CONF_FLAGS += $(patsubst CONFIG_%, -DCONFIG_%, $(filter CONFIG_%, $(.VARIABLES)))
CFLAGS += $(CONF_FLAGS)

.PHONY: all clean run iso version_h prepare

all: prepare version_h $(KERNEL_BIN)

include/config.h: .config
	@mkdir -p build include/generated
	@echo "  GEN       utsrelease.h"
	@echo "  GEN       include/config.h"
	@mkdir -p include
	$(Q)python3 scripts/gen_config.py

prepare:
	@mkdir -p build include/generated
	@echo "  GEN       utsrelease.h"
	@echo "#define BLUEOS_VERSION \"0.1.0-alpha\"" > include/generated/utsrelease.h
	@echo "#define BLUEOS_COMPILE_BY \"$(USER)\"" >> include/generated/utsrelease.h

version_h:
	@echo "  UPD       include/version.h"
	$(Q)sed -i 's/#define UTS_VERSION.*/#define UTS_VERSION    "#1 SMP PREEMPT '"$$(date +'%Y-%m-%d %H:%M:%S')"'"/' include/version.h 2>/dev/null || true

$(KERNEL_BIN): $(obj-y)
	@echo "  LD        $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $(obj-y) --whole-archive --no-whole-archive
	@echo "  DONE      BlueOS ($(ARCH)) is ready."

# Rules for C and Assembly
%.o: %.c
	@echo "  CC        $<"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	@echo "  AS        $<"
	$(Q)$(ASM) $(ASFLAGS) $< -o $@

%.o: %.s
	@echo "  AS_S      $<"
	$(Q)if [ "$(ARCH)" = "i386" ]; then $(ASM) $(ASFLAGS) $< -o $@; else $(CC) $(CFLAGS) -c $< -o $@; fi


# ISO (i386 only)
iso: all
ifeq ($(ARCH),i386)
	@echo "  MKISO     $(KERNEL_ISO)"
	@mkdir -p iso_root/boot/grub
	@cp $(KERNEL_BIN) iso_root/boot/$(KERNEL_BIN)
	@echo 'set timeout=0\nset default=0\nmenuentry "BlueOS" {\n  multiboot /boot/$(KERNEL_BIN)\n  boot\n}' > iso_root/boot/grub/grub.cfg
	$(Q)grub-mkrescue -o $(KERNEL_ISO) iso_root
	@rm -rf iso_root
else
	@echo "ISO generation only supported for i386"
endif

# Execution
run:
ifeq ($(ARCH),i386)
	$(Q)$(QEMU) -kernel $(KERNEL_BIN) -m 256M
else
	$(Q)$(QEMU) -nographic -machine virt -bios $(KERNEL_BIN)
endif

clean:
	@echo "  CLEAN     Objects and binaries"
	$(Q)rm -f $(KERNEL_BIN) $(KERNEL_ISO)
	$(Q)rm -rf build include/generated
	$(Q)find . -name "*.o" -type f -delete

menuconfig:
	@echo "Edita el archivo .config manualmente o usa KConfig-Conf si esta disponible"
	@nano .config
	$(MAKE) prepare
help:
	@echo "BlueOS Universal Build System"
	@echo "  make ARCH=i386       - Build for x86"
	@echo "  make ARCH=riscv      - Build for RISC-V"
	@echo "  make run             - Execute in QEMU"
	@echo "  make clean           - Wipe all build files"

initrd:
#	rm -f initrd.img init.bin
	dd if=/dev/zero of=initrd.img bs=1M count=10
	mkfs.fat -F 16 initrd.img
#	echo Hola_desde_el_disco > init.bin
#	mcopy -i initrd.img init.bin ::INIT.BIN
	hexdump -C initrd.img | head -n 15

initramfs.cpio: init.bin
	@echo "init.bin" | cpio -o -H newc > initramfs.cpio

init.bin: arch/i386/init.asm
	nasm -f bin arch/i386/init.asm -o init.bin

update_disk: init.bin
	@echo "Actualizando init.bin en la imagen de disco..."
	mcopy -o -i disk.img init.bin ::init.bin
	@echo "¡Listo!"
