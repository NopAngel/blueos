/*
 * arch/riscv/setup.c
 * Architecture-specific setup for RISC-V
 */

#include <kernel/printk.h>

extern void uart_init(void);

/**
 * arch_early_init - Configuración mínima para tener salida de texto
 */
void arch_early_init(void) {
  // uart_init();
}

/**
 * setup_arch - Configuración de interrupciones y registros de CPU
 */
void setup_arch(void) { boot_msg("Hardware: RISC-V Virt Machine detected.\n"); }

/**
 * mm_init - Inicialización del manejador de memoria (PMM)
 */
void mm_init(void) { boot_msg("Memory: Initializing PMM...\n"); }
