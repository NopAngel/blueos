/*
 * kernel/lkm.c
 * Native Kernel Module Loader & In-Memory Linker for BlueOS
 * Pure C, strictly in English.
 */

#include <elf.h>
#include <kernel/printk.h>
#include <lib/string.h>
#include <mm/memory.h>

/* Standard x86 ELF relocation types */
#define R_386_32   1  /* Direct 32-bit absolute reference */
#define R_386_PC32 2  /* PC-relative 32-bit reference */

/* A simple kernel symbol table registry matching names to actual memory addresses */
struct kernel_symbol {
    const char *name;
    uintptr_t address;
};

/* Registered internal kernel symbols available for dynamic module linking */
static struct kernel_symbol g_sys_symtab[] = {
    {"printk", (uintptr_t)printk},
    {"memcpy", (uintptr_t)memcpy},
    {"memset", (uintptr_t)memset},
    {NULL, 0}
};

/* Helper routine to locate the real kernel memory address of a symbol by name */
static uintptr_t find_kernel_symbol(const char *name) {
    for (int i = 0; g_sys_symtab[i].name != NULL; i++) {
        if (strcmp(g_sys_symtab[i].name, name) == 0) {
            return g_sys_symtab[i].address;
        }
    }
    return 0;
}

int lkm_load_module(void *module_buffer) {
    uint8_t *raw_elf = (uint8_t *)module_buffer;
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)raw_elf;

    /* Validate basic identity headers */
    if (ehdr->e_ident[0] != 0x7f || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        printk("<3> LKM ERROR: Invalid module ELF magic number!\n");
        return -1;
    }

    /* Must be an ELF relocatable file object type */
    if (ehdr->e_type != ET_REL) {
        printk("<3> LKM ERROR: Module file type is not relocatable (ET_REL)!\n");
        return -1;
    }

    Elf32_Shdr *shdr = (Elf32_Shdr *)(raw_elf + ehdr->e_shoff);
    const char *sh_strtab = (const char *)(raw_elf + shdr[ehdr->e_shstrndx].sh_offset);

    for (int i = 0; i < ehdr->e_shnum; i++) {
        /* Only allocate space for loadable execution flags or data sections */
        if ((shdr[i].sh_flags & SHF_ALLOC) && shdr[i].sh_size > 0) {
            
            /* Use BlueOS native frame allocator directly from pmm.c */
            extern void *pmm_alloc_frame(void);
            void *kernel_mem = pmm_alloc_frame();
            
            if (!kernel_mem) {
                printk("<3> LKM ERROR: Out of physical memory allocating section %s\n", sh_strtab + shdr[i].sh_name);
                return -1;
            }

            /* Bind the physical allocation and clear memory using internal tools */
            extern void *mm_memset(void *s, int c, size_t n);
            extern void *mm_memcpy(void *dest, const void *src, size_t n);

            if (shdr[i].sh_type == SHT_PROGBITS) {
                mm_memcpy(kernel_mem, raw_elf + shdr[i].sh_offset, shdr[i].sh_size);
            } else if (shdr[i].sh_type == SHT_NOBITS) {
                mm_memset(kernel_mem, 0, shdr[i].sh_size);
            }
            
            /* Overwrite the section address to register its new location inside the kernel */
            shdr[i].sh_addr = (uintptr_t)kernel_mem;
        }
    }

    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_REL) {
            /* Find target section being patched and symbol table reference identifiers */
            Elf32_Shdr *target_sec = &shdr[shdr[i].sh_info];
            Elf32_Shdr *symtab_sec = &shdr[shdr[i].sh_link];
            Elf32_Shdr *strtab_sec = &shdr[symtab_sec->sh_link];

            Elf32_Rel *rel = (Elf32_Rel *)(raw_elf + shdr[i].sh_offset);
            uint32_t rel_count = shdr[i].sh_size / sizeof(Elf32_Rel);

            Elf32_Sym *symtab = (Elf32_Sym *)(raw_elf + symtab_sec->sh_offset);
            const char *strtab = (const char *)(raw_elf + strtab_sec->sh_offset);

            for (uint32_t j = 0; j < rel_count; j++) {
                uint32_t sym_idx = ELF32_R_SYM(rel[j].r_info);
                uint32_t rel_type = ELF32_R_TYPE(rel[j].r_info);
                
                Elf32_Sym *sym = &symtab[sym_idx];
                uintptr_t sym_addr = 0;

                /* Calculate symbol reference target value destination */
                if (ELF32_ST_BIND(sym->st_info) == STB_LOCAL) {
                    /* Local symbol value depends on where its internal parent section got placed */
                    sym_addr = shdr[sym->st_shndx].sh_addr + sym->st_value;
                } else {
                    /* Global external dependency lookup (e.g. printk) */
                    const char *sym_name = strtab + sym->st_name;
                    sym_addr = find_kernel_symbol(sym_name);
                    if (!sym_addr) {
                        printk("<3> LKM LINK ERROR: Unresolved external symbol: %s\n", sym_name);
                        return -1;
                    }
                }

                /* Hardware patching interface targeting binary structures directly */
                uint32_t *patch_ptr = (uint32_t *)(target_sec->sh_addr + rel[j].r_offset);
                
                if (rel_type == R_386_32) {
                    /* Direct absolute patching calculation */
                    *patch_ptr += sym_addr;
                } else if (rel_type == R_386_PC32) {
                    /* PC-relative addressing calculation (essential for assembly CALL instructions) */
                    *patch_ptr += (sym_addr - (uintptr_t)patch_ptr);
                } else {
                    printk("<4> LKM WARN: Unsupported relocation opcode type %d\n", rel_type);
                }
            }
        }
    }

    printk("<6> LKM: All dynamic symbol relocations patched natively in memory.\n");
    
    /* Locate the module's initialization function from the ELF symbol tables */
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            Elf32_Sym *symtab = (Elf32_Sym *)(raw_elf + shdr[i].sh_offset);
            uint32_t sym_count = shdr[i].sh_size / sizeof(Elf32_Sym);
            const char *strtab = (const char *)(raw_elf + shdr[shdr[i].sh_link].sh_offset);

            for (uint32_t j = 0; j < sym_count; j++) {
                const char *sym_name = strtab + symtab[j].st_name;
                if (strcmp(sym_name, "module_init") == 0) {
                    /* Resolve pointer to the code section offset mapped into core RAM */
                    uintptr_t init_entry = shdr[symtab[j].st_shndx].sh_addr + symtab[j].st_value;
                    int (*init_func)(void) = (int (*)(void))init_entry;
                    
                    printk("<6> LKM: Executing module_init functional address entry at 0x%x\n", init_entry);
                    return init_func(); /* Execute the live loaded hardware module code */
                }
            }
        }
    }

    printk("<3> LKM ERROR: Entry routine 'module_init' target function missing from binary context.\n");
    return -1;
}