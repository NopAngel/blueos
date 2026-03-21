#include <drivers/keyboard.h>
#include <kernel/ports.h>

uint8_t arch_get_scancode() {
    return inb(0x60);
}

void arch_put_char(char c, unsigned int color) {
    // Here you put your SCREEN_BUFFER logic and scroll_screen()
}