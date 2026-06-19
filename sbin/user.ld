ENTRY(main)
SECTIONS
{
    . = 0x00500000;
    .text : { 
        *(.text)
        *(.text.*)
    }
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    .data : {
        *(.data)
        *(.data.*)
    }
    /DISCARD/ : { 
        *(.comment) 
        *(.note*) 
        *(.eh_frame)
    }
}