#!/usr/bin/env bash

MODULE_NAME="QEMU_AMD64"
KERNEL_ISO="build/blueos.iso"

echo "<6>[  ${MODULE_NAME}  ] Spinning up QEMU platform layer for PC-Legacy x86_64 execution..."

qemu-system-x86_64 \
    -cdrom "$KERNEL_ISO" \
    -m 512M \
    -smp 4 \
    -boot d \
    -serial stdio \
    -vga std \
    -no-reboot \
    -no-shutdown