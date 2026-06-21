/*
 * kernel/stack_protector.c
 * Stack Smashing Protector Core for BlueOS
 */

#include <stdint.h>

// Declaramos la función de pánico nativa de tu BlueOS
extern void k_panic(int code, const char *reason);

/* * El valor del canario. 
 * Para empezar usamos un número mágico reconocible (0xDEADC001).
 * Primero Dios, más adelante puedes usar el reloj del sistema o RDRAND 
 * para cambiarlo en cada arranque y que sea impredecible.
 */
uint32_t __stack_chk_guard = 0xDEADC001;

/*
 * Esta es la función que el compilador (GCC) busca y llama 
 * automáticamente si nota que el canario fue alterado antes de un return.
 */
__attribute__((noreturn)) void __stack_chk_fail(void) {
    // Código 13 (asociado clásicamente a fallos de protección general o traps de seguridad)
    k_panic(13, "STACK SMASHING DETECTED! Core stack guard compromised.");
    
    // Switch de seguridad por si k_panic alguna vez retorna
    while (1) {
        __asm__ volatile("cli; hlt");
    }
}