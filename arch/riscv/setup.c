/*
 * arch/riscv/setup.c
 * Architecture-specific setup for RISC-V
 */

#include <kernel/printk.h>

// Si tienes funciones de consola específicas para RISC-V, se inician aquí
extern void uart_init(void); 

/**
 * arch_early_init - Configuración mínima para tener salida de texto
 */
void arch_early_init(void) {
    // Aquí inicializas la UART o el driver de video básico
    // para que printk() funcione lo antes posible.
    // uart_init(); 
}

/**
 * setup_arch - Configuración de interrupciones y registros de CPU
 */
void setup_arch(void) {
    // Aquí configurarías los registros sstatus, stvec (traps), etc.
    pr_info("Hardware: RISC-V Virt Machine detected.\n");
}

/**
 * mm_init - Inicialización del manejador de memoria (PMM)
 */
void mm_init(void) {
    // Mueve aquí la lógica de memoria que tenías en init_fnc
    pr_info("Memory: Initializing PMM...\n");
}