.section .text
.global _start
.type _start, @function

_start:
    call main

1:  jmp 1b