ENTRY(main)
SECTIONS
{
    . = 0x00400000;

    .text : {
        *(.text .text.*)
    }

    .rodata : { *(.rodata .rodata.*) }
    .data   : { *(.data .data.*) }
    .bss    : { *(.bss) }

    /DISCARD/ :
    {
        *(.note.*)
        *(.comment)
    }
}