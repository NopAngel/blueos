#include <elf.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/kernel/types.h>
#include <sys/arch/x86/gdt.h>
#include <sys/mm/memory.h>

// Define the entry point function pointer type
typedef void (*entry_point)();

void load_elf_from_memory(uint8_t* buffer) {
    Elf32_Ehdr *header = (Elf32_Ehdr *)buffer;

    if (header->e_ident[EI_MAG0] != ELFMAG0 ||
        header->e_ident[EI_MAG1] != ELFMAG1 ||
        header->e_ident[EI_MAG2] != ELFMAG2 ||
        header->e_ident[EI_MAG3] != ELFMAG3) {
        // Not a valid ELF file
        return;
    }

    if (header->e_type != ET_EXEC) {
        // Not an executable file
        return;
    }

    if (header->e_machine != EM_386) {
        // Not for the x86 architecture
        return;
    }

    // Load program segments
    Elf32_Phdr *phdr = (Elf32_Phdr *)(buffer + header->e_phoff);
    for (int i = 0; i < header->e_phnum; ++i, ++phdr) {
        if (phdr->p_type == PT_LOAD) {
            // Allocate physical memory for the segment
            void *dest = (void *)phdr->p_vaddr;
            uint32_t mem_size = phdr->p_memsz;
            uint32_t file_size = phdr->p_filesz;

            // For simplicity, this example assumes virtual address equals physical address
            // and that there's enough space. A real implementation would need proper
            // virtual memory management (paging).
            if (mem_size > 0) {
                 // For this simplified example, we'll directly copy.
                 // A real kernel would allocate pages and map them.
                 memcpy(dest, buffer + phdr->p_offset, file_size);

                 // Zero out the rest of the segment if mem_size > file_size (BSS section)
                 if (mem_size > file_size) {
                     memset(dest + file_size, 0, mem_size - file_size);
                 }
            }
        }
    }

    // Jump to the entry point of the ELF file
    // The entry point is a virtual address. In this simple bootloader,
    // we assume we have an identity mapping, so we can just call it.
    entry_point start_user_code = (entry_point)header->e_entry;

    // Before jumping to user code, you would typically:
    // 1. Set up a user-mode stack.
    // 2. Switch to user mode (e.g., using an `iret` instruction).
    // For now, we will just call it directly for simplicity.
    start_user_code();
}
