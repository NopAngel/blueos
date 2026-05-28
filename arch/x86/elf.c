#include <elf.h>
#include <lib/string.h>
#include <kernel/types.h>
#include <arch/x86/gdt.h>
#include <mm/memory.h>
#include <kernel/printk.h>
#include <kernel/colors.h>

// The ELF magic number
#define EI_NIDENT 16
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

// ELF file types
#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4

// ELF machine types
#define EM_386 3

// Program header types
#define PT_LOAD 1

Elf32_Addr load_elf_from_memory(uint8_t* buffer) {
    Elf32_Ehdr *header = (Elf32_Ehdr *)buffer;

    // Check for the ELF magic number
    if (header->e_ident[0] != ELFMAG0 ||
        header->e_ident[1] != ELFMAG1 ||
        header->e_ident[2] != ELFMAG2 ||
        header->e_ident[3] != ELFMAG3) {
        printk(RED, "ELF: Invalid magic number!\n");
        return 0;
    }

    if (header->e_type != ET_EXEC) {
        printk(RED, "ELF: Not an executable file!\n");
        return 0;
    }

    if (header->e_machine != EM_386) {
        printk(RED, "ELF: Not for x86 architecture!\n");
        return 0;
    }

    // Load program segments
    Elf32_Phdr *phdr = (Elf32_Phdr *)(buffer + header->e_phoff);
    for (int i = 0; i < header->e_phnum; ++i, ++phdr) {
        if (phdr->p_type == PT_LOAD) {
            // A real implementation would use virtual memory management here.
            // For now, we assume virtual address equals physical address.
            void *dest = (void *)phdr->p_vaddr;
            
            // Copy the segment from the ELF file to its load address
            memcpy(dest, buffer + phdr->p_offset, phdr->p_filesz);

            // Zero out the BSS section (if any)
            if (phdr->p_memsz > phdr->p_filesz) {
                memset(dest + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
            }
        }
    }

    // Return the entry point of the ELF file
    return header->e_entry;
}
