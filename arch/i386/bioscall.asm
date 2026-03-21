[BITS 32]

section .text
global intcall

; struct biosregs {
;    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
;    uint16_t gs, fs, es, ds, eflags;
; };

intcall:
    push ebp
    mov ebp, esp
    pusha                   
    sidt [idt32_ptr]
    lidt [idt16_ptr]


    jmp 0x08:.compat16    

[BITS 16]
.compat16:
    mov eax, cr0
    and al, ~0x01
    mov cr0, eax

    jmp 0:.real_mode
    
.real_mode:
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov si, [ebp + 12]      

    mov al, [ebp + 8]       
    mov [.inst_int + 1], al
    
    mov eax, [si + 28]     
    mov ebx, [si + 16]      

.inst_int:
    int 0x00              
    mov eax, cr0
    or al, 0x01
    mov cr0, eax

    jmp 0x08:.back_to_32    

[BITS 32]
.back_to_32:
    lidt [idt32_ptr]        
    popa
    pop ebp
    ret

section .data
idt32_ptr: dw 0, 0, 0
idt16_ptr:
    dw 0x3FF                
    dd 0                   