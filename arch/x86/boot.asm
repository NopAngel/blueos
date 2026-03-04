[BITS 32]

section .multiboot
align 4
    dd 0x1BADB002            ; Magic
    dd 0x00                  ; Flags
    dd -(0x1BADB002 + 0x00)  ; Checksum


global boot
extern k_main


section .bss
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096
stack_bottom:
    resb 16384
stack_top:


section .rodata
align 16
gdt64:
    dq 0x0000000000000000    ; Null
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; Code
    dq (1<<44) | (1<<47) | (1<<41)           ; Data
gdt64_end:

gdt64_ptr:
    dw gdt64_end - gdt64 - 1
    dq gdt64


section .text
boot:

    mov esp, stack_top
    mov edi, ebx             

    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb no_long_mode
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz no_long_mode


    mov edi, p4_table
    xor eax, eax
    mov ecx, 4096 * 3
    rep stosb

    mov eax, p3_table
    or eax, 0b11
    mov [p4_table], eax

    mov eax, p2_table
    or eax, 0b11
    mov [p3_table], eax

    mov ecx, 0
map_p2_table:
    mov eax, 0x200000
    mul ecx
    or eax, 0b10000011
    mov [p2_table + ecx * 8], eax
    inc ecx
    cmp ecx, 512
    jne map_p2_table

    ; PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov eax, p4_table
    mov cr3, eax

   
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax


    lgdt [gdt64_ptr]
    jmp 0x08:init_64bit

no_long_mode:
    mov dword [0xb8000], 0x4f454f45 
    hlt

[BITS 64]
init_64bit:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov rdi, rbx 
    
    call k_main

halt_loop:
    hlt
    jmp halt_loop