#include <include/printk.h>
#include <include/colors.h>

extern int cursor_y;

void ___adddf3() {}
void ___subdf3() {}
void ___muldf3() {}
void ___divdf3() {}
void ___gtdf2()  {}
void ___ltdf2()  {}
void ___eqdf2()  {}
void ___fixdfsi() {}
void ___unorddf2() {}
void ___multf3() {}
void ___divtf3() {}

__attribute__((noreturn))



void k_panic(const char* msg, const char* file, int line) 
{
    
    bg_clear(WHITE_RED);
    cursor_y = 2; 


    printk("  __________________________________________________  ", cursor_y++, WHITE_RED);
    printk(" /                                                  \\ ", cursor_y++, WHITE_RED);
    printk(" |                !!! KERNEL PANIC !!!              | ", cursor_y++, WHITE_RED);
    printk(" \\__________________________________________________/ ", cursor_y++, WHITE_RED);
    
    cursor_y++; 

    printk("  REASON:", cursor_y++, WHITE_RED);
    printk(msg, cursor_y++, WHITE_RED);
    
    cursor_y++;

    printk("  LOCATION:", cursor_y++, WHITE_RED);
    printk("  File: ", cursor_y, WHITE_RED); 
    printk((char*)file, cursor_y++, WHITE_RED);
    

    
    cursor_y += 2;
    printk("  System halted. Please restart your computer.", cursor_y++, WHITE_RED);

    while(1) {
        __asm__ volatile ("cli; hlt");
    }
}