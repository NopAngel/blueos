ENTRY(_start)
SECTIONS
{
    . = 1M; /* El kernel empieza en 1MB */

    _start_kernel = .;

    .text : ALIGN(4K) {
        *(.multiboot)
        *(.text)
    }

    .rodata : ALIGN(4K) { *(.rodata) }
    .data   : ALIGN(4K) { *(.data) }
    .bss    : ALIGN(4K) { *(COMMON) *(.bss) }

    _end = .;
    end = .;

}
