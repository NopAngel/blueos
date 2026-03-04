OUTPUT_FORMAT(elf64-x86-64) /* Internamente trabajamos en 64 bits */
ENTRY(boot)

SECTIONS
{
    . = 1M; /* El kernel empieza en 1MB */

    .text : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata)
    }

    .data : ALIGN(4K)
    {
        *(.data)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
    _end = .; 

}

