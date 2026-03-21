ENTRY(_start)

SECTIONS
{
    . = 0x80000000;        


    .text : {
        KEEP(*(.text.boot))
        *(.text .text.*)

        
        . = ALIGN(8);
        __initcall_start = .;
        KEEP(*(.initcall0.init))
        KEEP(*(.initcall1.init))
        KEEP(*(.initcall2.init))
        KEEP(*(.initcall3.init))
        KEEP(*(.initcall4.init))
        KEEP(*(.initcall5.init))
        __initcall_end = .;
    }

    .rodata : {
        *(.rodata*)
        
        . = ALIGN(8);
        __initcall_start = .;
        KEEP(*(.initcall0.init)) /* pure */
        KEEP(*(.initcall1.init)) /* core */
        KEEP(*(.initcall2.init)) /* arch */
        KEEP(*(.initcall3.init)) /* fs */
        KEEP(*(.initcall4.init)) /* device */
        KEEP(*(.initcall5.init)) /* late */
        __initcall_end = .;
    }
    .data   : { *(.data) }

    .bss : {
        *(.bss)
        *(COMMON)
    }


    __jump_table : {
        __start___jump_table = .;
        *(__jump_table)
        __stop___jump_table = .;
    }
    
    . = ALIGN(4096);
    _end = .;           
}