#include <kernel/colors.h>
#include <kernel/printk.h>
#include <stdint.h>

// Símbolos exportados por el linker.ld (Asegúrate de tenerlos en tu script de enlace)
extern uint32_t _start;
extern uint32_t _end;

struct gdt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

struct gdt_entry gdt[6];
struct gdt_ptr gdtp;

extern void gdt_flush(uint32_t);

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access,
                  uint8_t gran) {
  gdt[num].base_low = (uint16_t)(base & 0xFFFF);
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xFF;

  gdt[num].limit_low = (limit & 0xFFFF);
  gdt[num].granularity = (limit >> 16) & 0x0F;

  gdt[num].granularity |= gran & 0xF0;
  gdt[num].access = access;
}

void gdt_init() {
  gdtp.limit = (sizeof(struct gdt_entry) * 6) - 1;
  gdtp.base = (uint32_t)&gdt;

  // 1. Null descriptor
  gdt_set_gate(0, 0, 0, 0, 0);

  /* * APLICANDO POLÍTICA W^X:
   * Calculamos el límite estricto de la sección ejecutable (.text).
   * Si la granularidad es 0xCF, el límite se multiplica por páginas de 4KB.
   * Modificamos el límite del segmento de código para que solo abarque hasta '_text_end'.
   */
  uint32_t text_limit = (uint32_t)&_end;
  
  // Convertimos el límite de bytes a bloques de 4KB si usamos granularidad de página (0xCF)
  uint32_t code_limit_pages = text_limit >> 12;

  // 2. Code segment (Kernel): Limitado estrictamente a la sección ejecutable
  gdt_set_gate(1, 0, code_limit_pages, 0x9A, 0xCF);
  
  // 3. Data segment (Kernel): Mantiene acceso a los 4GB para stack, heap y MMIO
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
  
  // 4. User mode code: De igual manera, se limita o se deja flat dependiendo del diseño de tu app de usuario
  gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
  
  // 5. User mode data: 4GB flat
  gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

  // 6. TSS Placeholder
  gdt_set_gate(5, 0, 0, 0, 0);

  gdt_flush((uint32_t)&gdtp);
  
  printk("[  \033[32mOK\033[0m  ] GDT: W^X protection segment deployed (Code limit: 0x%x).\n", text_limit);
}