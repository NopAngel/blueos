;
;   memory.asm 
;
;  (C) 2025 Angel Nieto/NopAngel <angelnieto1402@gmail.com>
;


bits 32
section .text

global mm_init
global mm_alloc
global mm_free
global mm_get_total
global mm_get_free
global mm_get_used


global memory_bitmap
global mm_total_pages
global mm_free_pages
global mm_next_free


mm_init:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    
    mov [multiboot_info], ebx
    
    mov edi, memory_bitmap
    mov ecx, 2048        
    xor eax, eax
    rep stosd           

    test ebx, ebx
    jz .use_default_memory
    
 
    mov eax, [ebx]
    test eax, 1 << 6     
    jz .use_default_memory

    mov eax, [ebx + 44]   
    mov ecx, [ebx + 48] 
    
    call process_memory_map
    jmp .init_done
    
.use_default_memory:
    call setup_default_memory
    
.init_done:

    call mark_kernel_memory
    
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

process_memory_map:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    
    mov esi, eax      
    lea ecx, [eax + ecx] 
    xor ebx, ebx       
    
.map_loop:
    cmp esi, ecx
    jge .map_done
    

    mov edx, [esi]      
    cmp edx, 20
    jb .next_entry
    
    
    mov eax, dword [esi + 16]  
    cmp eax, 1
    jne .next_entry   
    

    mov eax, dword [esi + 12]  
    add ebx, eax
    
.next_entry:
    add esi, edx        
    add esi, 4         
    jmp .map_loop
    
.map_done:

    mov [mm_total_bytes], ebx
    
    mov eax, ebx
    add eax, 4095
    shr eax, 12
    mov [mm_total_pages], eax
    
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret


setup_default_memory:
    push ebp
    mov ebp, esp
    
    ; 16MB = 16 * 1024 * 1024 = 16777216 bytes
    mov dword [mm_total_bytes], 16777216
    mov dword [mm_total_pages], 4096      ; 16MB / 4KB
    
    pop ebp
    ret


mark_kernel_memory:
    push ebp
    mov ebp, esp
    push ecx
    push edx
    
 
    mov ecx, 256        
    xor edx, edx        
    
.mark_kernel_loop:
    push dword 1       
    push edx            ; page
    call set_page_status
    add esp, 8          
    inc edx
    loop .mark_kernel_loop
    
    mov eax, [mm_total_pages]
    sub eax, 256
    mov [mm_free_pages], eax
    mov dword [mm_next_free], 256  ; fixed!
    
    pop edx
    pop ecx
    pop ebp
    ret

mm_alloc:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push edx
    push esi
    
    mov eax, ecx
    add eax, 4095
    shr eax, 12          
    mov esi, eax        
    
    mov edx, [mm_next_free]
    
.search_loop:
    mov eax, [mm_total_pages]
    sub eax, edx
    cmp eax, esi
    jl .alloc_fail
    
    push esi
    push edx
    call check_free_contiguous
    add esp, 8
    test eax, eax
    jnz .found_pages
    
    inc edx
    jmp .search_loop
    
.found_pages:

    mov ecx, esi
    mov ebx, edx
    
.mark_alloc_loop:
    push dword 1         
    push ebx             
    call set_page_status
    add esp, 8          
    inc ebx
    loop .mark_alloc_loop
    
    mov eax, [mm_free_pages]
    sub eax, esi
    mov [mm_free_pages], eax

    mov eax, edx
    shl eax, 12          ; * 4096
    
    add edx, esi
    mov [mm_next_free], edx
    
    jmp .alloc_done
    
.alloc_fail:
    xor eax, eax
    
.alloc_done:
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop ebp
    ret

mm_free:
    push ebp
    mov ebp, esp
    push eax
    push ebx
    push ecx
    push edx

    shr eax, 12          ; / 4096
    mov ebx, eax
    
    mov edx, ecx
    add edx, 4095
    shr edx, 12
    
    mov ecx, edx
    
.free_loop:
    push dword 0         
    push ebx            
    call set_page_status
    add esp, 8           
    inc ebx
    loop .free_loop
    
    mov eax, [mm_free_pages]
    add eax, edx
    mov [mm_free_pages], eax
    
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop ebp
    ret




set_page_status:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push edx
    
    mov ebx, [ebp + 8]   
    mov ecx, [ebp + 12] 
    

    mov eax, ebx
    shr eax, 3           ; byte = page / 8
    mov edx, ebx
    and edx, 7           ; bit = page % 8

    mov bl, byte [memory_bitmap + eax]  
    
    
    test ecx, ecx
    jz .clear_bit
    
    bts ebx, edx
    jmp .bit_done
    
.clear_bit:
    btr ebx, edx
    
.bit_done:
    mov byte [memory_bitmap + eax], bl  
    
    pop edx
    pop ecx
    pop ebx
    pop ebp
    ret


check_free_contiguous:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push edx
    push esi
    
    mov ebx, [ebp + 8]   
    mov ecx, [ebp + 12]  
    
.check_loop:
    push ebx
    call get_page_status
    add esp, 4
    
    test eax, eax
    jnz .not_free        
    
    inc ebx
    loop .check_loop
    
    ; Todas libres
    mov eax, 1
    jmp .done
    
.not_free:
    xor eax, eax
    
.done:
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop ebp
    ret


get_page_status:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    
    mov eax, [ebp + 8]   
    
    mov ebx, eax
    shr ebx, 3           ; byte
    and eax, 7           ; bit
    
    mov cl, byte [memory_bitmap + ebx] 
    
    bt ecx, eax
    setc al
    movzx eax, al
    
    pop ecx
    pop ebx
    pop ebp
    ret

mm_get_total:
    mov eax, dword [mm_total_bytes]  ; fix
    ret

mm_get_free:
    mov eax, dword [mm_free_pages]   ; fix
    shl eax, 12         
    ret

mm_get_used:
    mov eax, dword [mm_total_pages]  ; fix
    sub eax, dword [mm_free_pages]   ; fix
    shl eax, 12          ; * 4096
    ret


section .bss
align 4

memory_bitmap:
    resb 8192        

mm_total_bytes:   resd 1
mm_total_pages:   resd 1
mm_free_pages:    resd 1
mm_next_free:     resd 1
multiboot_info:   resd 1

section .data
align 4
