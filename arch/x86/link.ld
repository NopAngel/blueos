ENTRY(_start)
SECTIONS
{
    . = 1M; /* started in 1MB xd */

    _kernel_start = .;

    .text : ALIGN(4K) {
        *(.multiboot)
        *(.text)
    }

    .text : {
        *(.text)
        *(.text.*)
    }
    .data : {
        *(.data)
        *(.data.*)
    }

    .init :
    {
        . = ALIGN(4);
        _sysinit_start = .;
        *(.sysinit)
        _sysinit_end = .;
    }

    .rodata : ALIGN(4K) { *(.rodata) }
    .data   : ALIGN(4K) { *(.data) }
    .bss    : ALIGN(4K) { *(COMMON) *(.bss) }

    _end = .;
    end = .;
    _kernel_end = .;

}