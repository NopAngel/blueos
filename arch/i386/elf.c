#include <elf.h>
#include <fs/vfs.h>
#include <lib/string.h>
#include <blueos/printk.h>
extern uint8_t _binary_hello_elf_start[];
typedef void (*elf_entry_t)(void);

void load_elf_from_memory(uint8_t* buffer) {
    Elf32_Ehdr* header = (Elf32_Ehdr*)buffer;

    if (header->e_ident[0] != 0x7F || header->e_ident[1] != 'E') {
        return; 
    }

    Elf32_Phdr* phdr = (Elf32_Phdr*)(buffer + header->e_phoff);
    for (int i = 0; i < header->e_phnum; i++) {
        if (phdr[i].p_type == 1) { // PT_LOAD
            memcpy((void*)phdr[i].p_vaddr, buffer + phdr[i].p_offset, phdr[i].p_filesz);
            
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                memset((void*)(phdr[i].p_vaddr + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
            }
        }
    }

    void (*entry)() = (void (*)())header->e_entry;
    entry();
}

void load_elf_from_memory(uint8_t* buffer) {

    Elf32_Ehdr* header = (Elf32_Ehdr*)buffer;

    if (header->e_ident[0] != 0x7F || header->e_ident[1] != 'E' || 
        header->e_ident[2] != 'L' || header->e_ident[3] != 'F') {
        printk(RED, "ELF: Error - Invalid magic format\n");
        return;
    }

    printk(CYAN, "ELF: Validated file. Entry point: 0x%x\n", header->e_entry);

  
    Elf32_Phdr* phdr = (Elf32_Phdr*)(buffer + header->e_phoff);
    
    for (int i = 0; i < header->e_phnum; i++) {
   
        if (phdr[i].p_type == 1) {
            printk(WHITE, "ELF: Loading segment %d in 0x%x (%d bytes)\n", 
                   i, phdr[i].p_vaddr, phdr[i].p_memsz);

            uint8_t* dest = (uint8_t*)phdr[i].p_vaddr;
            uint8_t* src = buffer + phdr[i].p_offset;
            
            for (uint32_t j = 0; j < phdr[i].p_filesz; j++) {
                dest[j] = src[j];
            }

            for (uint32_t j = phdr[i].p_filesz; j < phdr[i].p_memsz; j++) {
                dest[j] = 0;
            }
        }
    }

    entry_point start_user_code = (entry_point)header->e_entry;

    __asm__ __volatile__("cli");

    start_user_code();

    printk(RED, "ELF: The program is returning. Blocking CPU.\n");
    while(1) {
        __asm__ __volatile__("hlt");
    }
}