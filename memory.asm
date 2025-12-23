;
;   memory.asm - Gestor de memoria básico para kernel
;
;  (C) 2025 Angel Nieto/NopAngel <angelnieto1402@gmail.com>
;
;           Este código tiene licencia APACHE 2.0
;

bits 32
section .text

global mm_init
global mm_alloc
global mm_free
global mm_get_total
global mm_get_free
global mm_get_used

; Variables exportadas
global memory_bitmap
global mm_total_pages
global mm_free_pages
global mm_next_free

;----------------------------------------------------------------
; mm_init - Inicializa el gestor de memoria
; Input: EBX = puntero a info multiboot
;----------------------------------------------------------------
mm_init:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    
    ; Guardar info multiboot
    mov [multiboot_info], ebx
    
    ; Inicializar bitmap a 0 (toda memoria libre)
    mov edi, memory_bitmap
    mov ecx, 2048        ; 2048 dwords = 8192 bytes
    xor eax, eax
    rep stosd           ; CORREGIDO: usar stosd en lugar de stosb
    
    ; Procesar mapa de memoria multiboot si existe
    test ebx, ebx
    jz .use_default_memory
    
    ; Verificar flags de multiboot
    mov eax, [ebx]
    test eax, 1 << 6     ; bit 6 = memoria map disponible
    jz .use_default_memory
    
    ; Obtener mapa de memoria
    mov eax, [ebx + 44]   ; mmap_addr
    mov ecx, [ebx + 48]   ; mmap_length
    
    call process_memory_map
    jmp .init_done
    
.use_default_memory:
    ; Asumir 16MB de RAM si no hay info multiboot
    call setup_default_memory
    
.init_done:
    ; Marcar primeras páginas como ocupadas (kernel)
    call mark_kernel_memory
    
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

;----------------------------------------------------------------
; process_memory_map - Procesa el mapa de memoria de multiboot
; Input: EAX = puntero al mapa, ECX = longitud
;----------------------------------------------------------------
process_memory_map:
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi
    
    mov esi, eax        ; ESI = puntero al mapa
    lea ecx, [eax + ecx] ; ECX = fin del mapa (CORREGIDO)
    xor ebx, ebx        ; EBX = total de memoria usable
    
.map_loop:
    cmp esi, ecx
    jge .map_done
    
    ; Obtener tamaño de la entrada
    mov edx, [esi]      ; tamaño de entrada
    cmp edx, 20
    jb .next_entry
    
    ; Obtener tipo de memoria (1 = usable)
    mov eax, dword [esi + 16]  ; CORREGIDO: especificar tamaño
    cmp eax, 1
    jne .next_entry     ; Cambiado de .reserved_memory a .next_entry
    
    ; Sumar memoria usable
    mov eax, dword [esi + 12]  ; length baja (CORREGIDO)
    add ebx, eax
    
.next_entry:
    ; Siguiente entrada
    add esi, edx        ; tamaño de entrada
    add esi, 4          ; +4 por el tamaño incluido
    jmp .map_loop
    
.map_done:
    ; Guardar memoria total (en bytes)
    mov [mm_total_bytes], ebx
    
    ; Calcular páginas totales
    mov eax, ebx
    add eax, 4095
    shr eax, 12
    mov [mm_total_pages], eax
    
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

;----------------------------------------------------------------
; setup_default_memory - Configura 16MB por defecto
;----------------------------------------------------------------
setup_default_memory:
    push ebp
    mov ebp, esp
    
    ; 16MB = 16 * 1024 * 1024 = 16777216 bytes
    mov dword [mm_total_bytes], 16777216
    mov dword [mm_total_pages], 4096      ; 16MB / 4KB
    
    pop ebp
    ret

;----------------------------------------------------------------
; mark_kernel_memory - Marca memoria del kernel como ocupada
;----------------------------------------------------------------
mark_kernel_memory:
    push ebp
    mov ebp, esp
    push ecx
    push edx
    
    ; Kernel ocupa aproximadamente primeros 1MB
    ; 1MB = 256 páginas de 4KB
    mov ecx, 256        ; páginas del kernel
    xor edx, edx        ; empezar desde página 0
    
.mark_kernel_loop:
    push dword 1        ; ocupar (CORREGIDO)
    push edx            ; página
    call set_page_status
    add esp, 8          ; limpiar pila
    inc edx
    loop .mark_kernel_loop
    
    ; Actualizar contadores
    mov eax, [mm_total_pages]
    sub eax, 256
    mov [mm_free_pages], eax
    mov dword [mm_next_free], 256  ; CORREGIDO
    
    pop edx
    pop ecx
    pop ebp
    ret

;----------------------------------------------------------------
; mm_alloc - Asigna memoria física
; Input:  ECX = tamaño en bytes
; Output: EAX = dirección física, 0 si error
;----------------------------------------------------------------
mm_alloc:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push edx
    push esi
    
    ; Calcular páginas necesarias
    mov eax, ecx
    add eax, 4095
    shr eax, 12          ; EAX = páginas necesarias
    mov esi, eax         ; ESI guarda el número de páginas
    
    ; Buscar páginas consecutivas libres
    mov edx, [mm_next_free]
    
.search_loop:
    ; Verificar si tenemos espacio suficiente
    mov eax, [mm_total_pages]
    sub eax, edx
    cmp eax, esi
    jl .alloc_fail
    
    ; Verificar si ESI páginas están libres empezando en EDX
    push esi
    push edx
    call check_free_contiguous
    add esp, 8
    test eax, eax
    jnz .found_pages
    
    ; Siguiente página
    inc edx
    jmp .search_loop
    
.found_pages:
    ; Marcar páginas como ocupadas
    mov ecx, esi
    mov ebx, edx
    
.mark_alloc_loop:
    push dword 1         ; ocupar (CORREGIDO)
    push ebx             ; página
    call set_page_status
    add esp, 8           ; limpiar pila
    inc ebx
    loop .mark_alloc_loop
    
    ; Actualizar contadores
    mov eax, [mm_free_pages]
    sub eax, esi
    mov [mm_free_pages], eax
    
    ; Calcular dirección física
    mov eax, edx
    shl eax, 12          ; * 4096
    
    ; Actualizar siguiente página libre
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

;----------------------------------------------------------------
; mm_free - Libera memoria física
; Input:  EAX = dirección física, ECX = tamaño en bytes
;----------------------------------------------------------------
mm_free:
    push ebp
    mov ebp, esp
    push eax
    push ebx
    push ecx
    push edx
    
    ; Calcular página inicial
    shr eax, 12          ; / 4096
    mov ebx, eax
    
    ; Calcular páginas a liberar
    mov edx, ecx
    add edx, 4095
    shr edx, 12
    
    ; Marcar páginas como libres
    mov ecx, edx
    
.free_loop:
    push dword 0         ; liberar (CORREGIDO)
    push ebx             ; página
    call set_page_status
    add esp, 8           ; limpiar pila
    inc ebx
    loop .free_loop
    
    ; Actualizar contador de páginas libres
    mov eax, [mm_free_pages]
    add eax, edx
    mov [mm_free_pages], eax
    
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop ebp
    ret

;----------------------------------------------------------------
; Funciones auxiliares - CORREGIDAS
;----------------------------------------------------------------

; set_page_status - Cambia estado de una página
; Input: Pila: página (dword), estado (dword: 0=libre, 1=ocupada)
set_page_status:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push edx
    
    mov ebx, [ebp + 8]   ; número de página (CORREGIDO: primer argumento)
    mov ecx, [ebp + 12]  ; estado (CORREGIDO: segundo argumento)
    
    ; Calcular byte y bit en bitmap
    mov eax, ebx
    shr eax, 3           ; byte = página / 8
    mov edx, ebx
    and edx, 7           ; bit = página % 8
    
    ; Obtener byte actual
    mov bl, byte [memory_bitmap + eax]  ; CORREGIDO: especificar tamaño byte
    
    ; Cambiar bit según estado
    test ecx, ecx
    jz .clear_bit
    
    ; Establecer bit
    bts ebx, edx
    jmp .bit_done
    
.clear_bit:
    btr ebx, edx
    
.bit_done:
    mov byte [memory_bitmap + eax], bl  ; CORREGIDO: especificar tamaño
    
    pop edx
    pop ecx
    pop ebx
    pop ebp
    ret

; check_free_contiguous - Verifica páginas consecutivas libres
; Input: Pila: página inicial (dword), número de páginas (dword)
; Output: EAX = 1 si libres, 0 si ocupadas
check_free_contiguous:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push edx
    push esi
    
    mov ebx, [ebp + 8]   ; página inicial
    mov ecx, [ebp + 12]  ; número de páginas
    
.check_loop:
    ; Verificar si la página está libre
    push ebx
    call get_page_status
    add esp, 4
    
    test eax, eax
    jnz .not_free        ; si está ocupada
    
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

; get_page_status - Obtiene estado de una página
; Input: Pila: página (dword)
; Output: EAX = 0 si libre, 1 si ocupada
get_page_status:
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    
    mov eax, [ebp + 8]   ; número de página
    
    ; Calcular byte y bit
    mov ebx, eax
    shr ebx, 3           ; byte
    and eax, 7           ; bit
    
    ; Obtener byte
    mov cl, byte [memory_bitmap + ebx]  ; CORREGIDO: especificar tamaño
    
    ; Extraer bit
    bt ecx, eax
    setc al
    movzx eax, al
    
    pop ecx
    pop ebx
    pop ebp
    ret

; Funciones de consulta
mm_get_total:
    mov eax, dword [mm_total_bytes]  ; CORREGIDO
    ret

mm_get_free:
    mov eax, dword [mm_free_pages]   ; CORREGIDO
    shl eax, 12          ; * 4096 para obtener bytes
    ret

mm_get_used:
    mov eax, dword [mm_total_pages]  ; CORREGIDO
    sub eax, dword [mm_free_pages]   ; CORREGIDO
    shl eax, 12          ; * 4096
    ret

;----------------------------------------------------------------
; Sección de datos
;----------------------------------------------------------------
section .bss
align 4

; Bitmap de memoria (1 bit por página de 4KB)
memory_bitmap:
    resb 8192        ; 8192 bytes para bitmap (soporta hasta 64MB)

; Variables del gestor
mm_total_bytes:   resd 1
mm_total_pages:   resd 1
mm_free_pages:    resd 1
mm_next_free:     resd 1
multiboot_info:   resd 1

section .data
align 4
