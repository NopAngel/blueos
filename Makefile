#
# BlueOS Universal Makefile
#     (B U M)
#

-include .config

# --- Basic Configuration ---
ARCH ?= x86
V ?= 0
ifeq ($(V),1)
  Q =
else
  Q = @
endif

# --- Toolchain & Flags Setup ---
ifeq ($(ARCH),x86)
    # x86 Specific
    CC      = x86_64-linux-gnu-gcc
    ASM     = nasm
    LD      = ld
    QEMU    = qemu-system-x86_64
    GDB     = gdb

    GCC_INC = $(shell $(CC) -print-file-name=include)
    CFLAGS  = -m32 -march=i386 -mno-sse -mno-sse2 -mno-mmx -ffreestanding -fno-builtin -std=gnu99 -nostdlib \
              -fno-stack-protector -nostdinc -fno-pic -I. -Iinclude -I$(GCC_INC) -O2 \
              -Dx86 -D__x86__
    ASFLAGS = -f elf32
    LDFLAGS = -m elf_i386 -T arch/x86/link.ld -z noexecstack

    KERNEL_BIN = blueos.elf
    KERNEL_ISO = blueos.iso
    USER_ELF   = hello.elf
    RUST_TARGET = i686-unknown-linux-gnu
    RUST_LIB = rust/target/$(RUST_TARGET)/release/librust.a

    # Objects for x86
    obj-y := arch/x86/boot.o kernel/printk.o init/all.o arch/x86/panic.o kernel/task.o usr/lib/string.o \
             arch/x86/interrupts.o arch/x86/interrupt_entry.o arch/x86/profile.o usr/auth.o \
             arch/x86/switch.o arch/x86/bg.o drivers/lru/multilru.o \
             fs/help.o drivers/tty/tty.o drivers/input/keyboard/8042/keyboard.o drivers/input/keyboard/kbd_common.o arch/x86/syscall.o \
             drivers/scsi/scsi_core.o arch/common/pic.o drivers/scsi/scsi_lsi.o drivers/video/ansi_tty.o fs/tmpfs/tmpfs.o \
             drivers/pci/pci.o drivers/pinctrl/pinctrl.o arch/x86/idt.o kernel/ping.o net/stack.o \
             arch/x86/irq.o kernel/sysctl.o fs/vboxfs/vboxfs.o arch/x86/keyboard_io.o \
             fs/9p/9p.o kernel/vmcore_info.o drivers/leds/leds.o drivers/vhost/vhost_net.o \
             arch/x86/apic.o arch/x86/kvm.o kernel/hlec.o arch/x86/timer.o kernel/hpet.o arch/x86/shell.o \
             drivers/rtc/rtc.o drivers/battery/battery.o arch/common/memory.o arch/x86/acpi.o \
             init/kernel.o arch/x86/intel.o arch/x86/amd.o arch/x86/interrupts-a.o drivers/net/virtio_net.o \
             drivers/isapnp/isapnp.o arch/x86/isr.o drivers/usb/usb.o drivers/dma/hdc_dma.o crypto/sha256.o drivers/bcma/bcma.o \
             kernel/module.o usr/bluefetch.o kernel/ksyms.o fs/vfs/vfs.o usr/qsh.o \
             arch/x86/gdt.o arch/x86/gdt-a.o arch/x86/virt.o drivers/disk/disk.o \
             fs/ramfs/fs.o sound/sdw/s.o lib/network.o drivers/video/vt220.o drivers/video/vt100.o \
             drivers/ata/ata.o init/hyper.o arch/x86/cmdline.o drivers/amba/amba_pl011.o drivers/video/virtio.o \
             drivers/virtio/virtio_9p.o usr/commands.o drivers/video/serial.o kernel/time.o arch/x86/vmm.o \
             drivers/connector/connector.o drivers/i2c/i2c.o drivers/thermal/lm75.o drivers/soc/soc_intel.o \
             drivers/pnp/pnp.o drivers/core/live_config.o arch/x86/pm.o kernel/mm/malloc.o drivers/cdrom/cdrom.o drivers/hyperv/hypervisor.o \
             arch/x86/elf.o kernel/userspace.o arch/x86/userspace_a.o arch/x86/loader.o kernel/mm/pmm.o \
             arch/common/hal.o drivers/gpio/gpio.o fs/ext2/ext2.o arch/x86/arch.o kernel/sched.o init/version.o usr/lib/syscall_wrapper.o drivers/input/joystick/ps3/ps3_ds3.o drivers/input/mouse/8042/mouse.o

else ifeq ($(ARCH),riscv)
    # RISC-V Specific
    CC      = riscv64-unknown-elf-gcc
    AS      = riscv64-unknown-elf-as
    LD      = ld
    QEMU    = qemu-system-riscv32

    GCC_INC = $(shell $(CC) -print-file-name=include)
    INCLUDES = -Iinclude -Iinclude/kernel -Iinclude/arch/riscv -Iinclude/drivers -I$(GCC_INC) -I.
    CFLAGS  = -march=rv32i_zicsr -mabi=ilp32 -ffreestanding -fno-builtin -std=gnu99 -nostdlib \
              -fno-stack-protector -nostdinc -fno-pic -O0 -g -DRISCV $(INCLUDES) \
              -march=rv32imafdc_h_zicsr_zifencei_zbkb_zbkc_zbkx_zknd_zkne_zknh
    ASFLAGS = -march=rv32i_zicsr -mabi=ilp32 -g
    LDFLAGS = -m elf32lriscv -T arch/riscv/link.ld

    KERNEL_BIN = blueos.elf

    # Objects for RISC-V
    obj-y := arch/riscv/boot.o init/kernel.o kernel/printk.o arch/riscv/acpi.o \
             arch/riscv/plic.o arch/riscv/trap.o arch/riscv/trap_entry.o arch/riscv/trap_handler.o \
             arch/riscv/interrupts.o arch/riscv/panic.o usr/commands.o drivers/input/keyboard/i8259A/keyboard.o arch/riscv/keyboard_io.o drivers/gpio/gpio.o \
             arch/riscv/shell.o arch/riscv/profile.o arch/riscv/kvm.o arch/common/memory.o \
             arch/riscv/vendor_h.o arch/riscv/intel_h.o init/all.o kernel/task.o \
             arch/riscv/switch.o drivers/lru/multilru.o drivers/vhost/vhost_net.o usr/bluefetch.o \
             drivers/battery/battery.o drivers/bcma/bcma.o drivers/connector/connector.o \
             drivers/pnp/pnp.o drivers/core/live_config.o fs/ramfs/fs.o fs/eventfs.o \
             usr/lib/string.o crypto/sha256.o usr/auth.o kernel/sched.o arch/riscv/uart.o \
             arch/riscv/timer.o kernel/mm/vmm.o lib/list.o \
             crypto/aes.o crypto/aes_riscv_glue.o crypto/sm3_riscv_glue.o arch/riscv/fpu.o \
             kernel/power/hibernate.o arch/riscv/hibernate_asm.o arch/riscv/intr.o \
             drivers/power/power.o kernel/jump_label.o arch/riscv/kvm/nmu.o \
             kernel/net/net.o kernel/bpf/jit_rv64.o kernel/mm/fault.o kernel/process.o \
             kernel/mm/demand_paging.o kernel/mm/pmm.o kernel/mm/ptdump.o drivers/xen/xen.o \
             drivers/watchdog/wdt.o drivers/zorro/zorro.o drivers/android/binder.o \
             drivers/android/ashmem.o drivers/mailbox/mbox.o drivers/input/joystick/ps3/ps3_ds3.o \
             drivers/tc/tc.o drivers/cxl/cxl_m.o drivers/ptp/ptp.o drivers/pps/pps.o \
             drivers/firmware/firmware.o drivers/w1/w1.o drivers/hal/gpio.o \
             drivers/hal/timer.o drivers/dax/dax.o drivers/edac/edac.o drivers/uio/uio.o \
             drivers/macintosh/adb.o drivers/hx/haptics.o drivers/i2c/i2c.o \
             kernel/mm/malloc.o arch/common/hal.o fs/vfs/vfs.o drivers/leds/leds.o \
             drivers/pinctrl/pinctrl.o drivers/scsi/scsi_core.o drivers/scsi/scsi_lsi.o \
             drivers/isapnp/isapnp.o init/version.o usr/lib/syscall_wrapper.o
endif

# --- Global Logic ---
CONF_FLAGS += $(patsubst CONFIG_%, -DCONFIG_%, $(filter CONFIG_%, $(.VARIABLES)))
CFLAGS += $(CONF_FLAGS)

.PHONY: all clean run iso version_h prepare

all: prepare version_h
ifeq ($(ARCH),x86)
	$(Q)$(MAKE) $(USER_ELF)
endif
	$(Q)$(MAKE) $(KERNEL_BIN)

include/config.h: .config
	@mkdir -p build include/generated
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

$(USER_ELF): bin/hello_elf.c
	@echo "  CC_USER   $<"
	$(Q)$(CC) -m32 -fno-stack-protector -fno-pie -ffreestanding -nostdlib -static -Ttext 0x1000000 $< -o $@


# --- Rules for C and Assembly ---
%.o: %.c include/generated/utsrelease.h
	@echo "  CC        $<"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	@echo "  AS        $<"
	$(Q)$(ASM) $(ASFLAGS) $< -o $@

%.o: %.S
	@echo "  AS_S      $<"
ifeq ($(ARCH),x86)
	$(Q)$(ASM) $(ASFLAGS) $< -o $@
else
	$(Q)$(CC) $(CFLAGS) -c $< -o $@
endif

%.o: %.s
	@echo "  AS_s      $<"
ifeq ($(ARCH),x86)
	$(Q)$(ASM) $(ASFLAGS) $< -o $@
else
	$(Q)$(AS) $(ASFLAGS) $< -o $@
endif

# ISO (x86 only)
iso: all
ifeq ($(ARCH),x86)
	@echo "  MKISO      $(KERNEL_ISO)"
	@mkdir -p iso_root/boot/grub
	@cp $(KERNEL_BIN) iso_root/boot/$(KERNEL_BIN)
	@echo -e 'set timeout=0\nset default=0\nmenuentry "BlueOS" {\n  multiboot /boot/$(KERNEL_BIN)\n  boot\n}' > iso_root/boot/grub/grub.cfg
	$(Q)grub-mkrescue -o $(KERNEL_ISO) iso_root
	@rm -rf iso_root
else
	@echo "ISO generation only supported for x86"
endif

# RUN (with QEMU)
run:
ifeq ($(ARCH),x86)
	$(Q)$(QEMU) -kernel $(KERNEL_BIN) -m 256M
else
	$(Q)$(QEMU) -nographic -machine virt -bios $(KERNEL_BIN)
endif

# Clean
clean:
	@echo "  CLEAN      Objects and binaries"
	$(Q)rm -f $(KERNEL_BIN) $(KERNEL_ISO) $(USER_ELF)
	$(Q)rm -rf build include/generated
	$(Q)find . -name "*.o" -type f -delete
	@echo "  DONE       Clean completed."

menuconfig:
	@nano .config
	$(MAKE) prepare

help:
	@echo "             BUBS Makefile   "
	@echo "  (BlueOS Universal Build System)"
	@echo "  make ARCH=x86        - Build for x86"
	@echo "  make ARCH=riscv      - Build for RISC-V"
	@echo "  make run             - Execute in QEMU"
	@echo "  make clean           - Wipe all build files"
	@echo "  make iso             - Create bootable ISO (x86 only)"
