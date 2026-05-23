ENTRY(_start)
SECTIONS
{
    . = 1M; /* started in 1MB xd */

    _kernel_start = .;

    .text : ALIGN(4K) {
        *(.multiboot)
        *(.text)
    }

    .rodata : ALIGN(4K) { *(.rodata) }
    .data   : ALIGN(4K) { *(.data) }
    .bss    : ALIGN(4K) { *(COMMON) *(.bss) }

    _end = .;
    end = .;
    _kernel_end = .;

}
