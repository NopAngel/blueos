#include <kernel/ports.h>
#include <stdint.h>

void play_sound(uint32_t n_frequence) {
    if (n_frequence == 0) return;

    uint32_t div = 1193182 / n_frequence;
    
    // 1. Bloqueamos interrupciones para tener control total del bus
    disable_interrupts();

    // 2. Configurar el PIT Canal 2 (Modo 3: Onda cuadrada)
    outb(0x43, 0xB6);
    io_wait(); // Pequeña espera para hardware lento

    // 3. Enviar el divisor (LSB luego MSB)
    outb(0x42, (uint8_t)(div & 0xFF));
    io_wait();
    outb(0x42, (uint8_t)((div >> 8) & 0xFF));

    // 4. Activar los bits del Speaker en el puerto 0x61
    uint8_t tmp = inb(0x61);
    if ((tmp & 3) != 3) {
        outb(0x61, tmp | 3);
    }

    // 5. Devolvemos el control al sistema
    enable_interrupts();
}

void nosound() {
    // Desactivamos los bits 0 y 1 manteniendo el resto igual
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}