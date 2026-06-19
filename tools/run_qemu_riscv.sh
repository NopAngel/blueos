#!/usr/bin/env bash

MODULE_NAME="QEMU_RISCV"
KERNEL_IMAGE="arch/riscv/boot/Image"

if [ ! -f "$KERNEL_IMAGE" ]; then
    echo "<3>[  ${MODULE_NAME}  ] Error: Target compiled structural boot image binary missing."
    exit 1
fi

echo "<6>[  ${MODULE_NAME}  ] Spinning up QEMU Virt machine environment for RISC-V 64-bit target..."

qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -smp 2 \
    -m 256M \
    -kernel "$KERNEL_IMAGE" \
    -nographic \
    -bios default