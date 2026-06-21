/*
 * include/elf.h
 * Extended Executable and Linkable Format (ELF) Definitions for BlueOS
 * Pure C, strictly in English.
 */

#ifndef _BLUEOS_ELF_H
#define _BLUEOS_ELF_H

#include <stdint.h>

/* ELF Base Types */
typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf32_Word;

/* ELF File Header (Ehdr) */
#define EI_NIDENT 16
typedef struct {
    uint8_t      e_ident[EI_NIDENT];
    Elf32_Half   e_type;
    Elf32_Half   e_machine;
    Elf32_Word   e_version;
    Elf32_Addr   e_entry;
    Elf32_Off    e_phoff;
    Elf32_Off    e_shoff;
    Elf32_Word   e_flags;
    Elf32_Half   e_ehsize;
    Elf32_Half   e_phentsize;
    Elf32_Half   e_phnum;
    Elf32_Half   e_shentsize;
    Elf32_Half   e_shnum;
    Elf32_Half   e_shstrndx;
} Elf32_Ehdr;

/* ELF File Types */
#define ET_NONE      0
#define ET_REL       1  /* Relocatable file object */
#define ET_EXEC      2  /* Executable file */
#define ET_DYN       3
#define ET_CORE      4

/* ELF Machine Architectures */
#define EM_386       3  /* Intel 80386 */

/* ELF Program Header (Phdr) */
typedef struct {
    Elf32_Word   p_type;
    Elf32_Off    p_offset;
    Elf32_Addr   p_vaddr;
    Elf32_Addr   p_paddr;
    Elf32_Word   p_filesz;
    Elf32_Word   p_memsz;
    Elf32_Word   p_flags;
    Elf32_Word   p_align;
} Elf32_Phdr;

#define PT_LOAD      1

/* ELF Section Header (Shdr) */
typedef struct {
    Elf32_Word   sh_name;
    Elf32_Word   sh_type;
    Elf32_Word   sh_flags;
    Elf32_Addr   sh_addr;
    Elf32_Off    sh_offset;
    Elf32_Word   sh_size;
    Elf32_Word   sh_link;
    Elf32_Word   sh_info;
    Elf32_Word   sh_addralign;
    Elf32_Word   sh_entsize;
} Elf32_Shdr;

/* Section Types (sh_type) */
#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_HASH     5
#define SHT_DYNAMIC  6
#define SHT_NOTE     7
#define SHT_NOBITS   8
#define SHT_REL      9

/* Section Attribute Flags (sh_flags) */
#define SHF_WRITE    0x1
#define SHF_ALLOC    0x2
#define SHF_EXECINSTR 0x4

/* ELF Symbol Table Entry (Sym) */
typedef struct {
    Elf32_Word   st_name;
    Elf32_Addr   st_value;
    Elf32_Word   st_size;
    uint8_t      st_info;
    uint8_t      st_other;
    Elf32_Half   st_shndx;
} Elf32_Sym;

/* Symbol Binding Macros */
#define ELF32_ST_BIND(info)   ((info) >> 4)
#define ELF32_ST_TYPE(info)   ((info) & 0xf)
#define STB_LOCAL             0
#define STB_GLOBAL            1

/* ELF Relocation Entry without explicit addend (Rel) */
typedef struct {
    Elf32_Addr   r_offset;
    Elf32_Word   r_info;
} Elf32_Rel;

/* Relocation Macros */
#define ELF32_R_SYM(info)     ((info) >> 8)
#define ELF32_R_TYPE(info)    ((info) & 0xff)

#endif /* _BLUEOS_ELF_H */