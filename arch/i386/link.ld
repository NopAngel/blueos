
ENTRY(_start)

SECTIONS
{
    /* El estándar para kernels de 32 bits es empezar en 1MB */
    . = 0x100000; 

    .text : {
        *(.multiboot) /* ¡Esto debe ir de primero! */
        *(.text)
    }

    .rodata : { *(.rodata) }
    .data   : { *(.data) }
    .bss    : { *(.bss) }

    _end = .;    /* <--- ESTA ES LA CLAVE */
    end = .;     /* Ponemos ambas por si acaso */
}