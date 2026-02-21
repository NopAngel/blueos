#include <include/elf.h>
#include <include/printk.h>
#include <include/colors.h> 
#include <include/panic.h> 
#include <include/string/string.h>

typedef void (*entry_point_t)(void);

void load_elf(void* elf_data) {
    Elf32_Ehdr* header = (Elf32_Ehdr*)elf_data;

    uint32_t entry_point = header->e_entry;

    printk(GREEN, "Ejecutando ELF en 0x%x...\n", entry_point);
    if (*(uint32_t*)header->e_ident != ELF_MAGIC) {
        PANIC("Archivo no es un ELF valido");
    }

    Elf32_Phdr* phdr = (Elf32_Phdr*)((uint32_t)elf_data + header->e_phoff);
    
    for (int i = 0; i < header->e_phnum; i++) {
        if (phdr[i].p_type == 1) { 
      
            memcpy((void*)phdr[i].p_vaddr, 
                   (void*)((uint32_t)elf_data + phdr[i].p_offset), 
                   phdr[i].p_filesz);
            
        
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                memset((void*)(phdr[i].p_vaddr + phdr[i].p_filesz), 
                       0, phdr[i].p_memsz - phdr[i].p_filesz);
            }
        }
    }

    printk(GREEN, "Saltando al punto de entrada ELF: 0x%x\n", header->e_entry);
    entry_point_t entry = (entry_point_t)header->e_entry;
    __asm__ volatile (
        "mov %0, %%ebx;"    
        "jmp *%1;"          
        : 
        : "r"(0x12345678), "r"(entry_point) 
        : "ebx"
    );
}