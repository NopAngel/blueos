// arch/kernel.c
#include <include/colors.h>
#include <include/printk.h>
#include <include/ports.h>
#include <include/panic.h>
#include <include/init_fnc.h>
#include <include/drivers/keyboard.h>
#include <include/fs/vfs.h>
#include <include/fs/fs.h>
#include <include/version.h>
#include <include/gui/dosbox/main.h>
#include <include/gui/dosbox/utils.h>
#include <include/interrupts.h>
int cursor_x = 0;
int cursor_y = 0;


//RUST



extern int is_command_safe(const char* cmd, int len);
extern int suma_rust(int a, int b) __asm__("suma_rust");
extern void loadpin_init(int device_id);
extern int loadpin_check(int device_id);
extern void __stack_chk_fail(void);
extern int ipe_verify_binary(const unsigned char* buffer, int size);
extern int landlock_restrict(int pid, unsigned int min, unsigned int max, int io);
extern int landlock_check_mem(int pid, unsigned int addr);

void kernel_write_mem(int pid, unsigned int addr, char value) {
    if (landlock_check_mem(pid, addr)) {
        *(char*)addr = value;
    } else {
        printk("LANDLOCK: Bloqueado acceso ilegal a memoria del PID %d!", pid, RED);
       
    }
}




__attribute__((visibility("hidden"))) 
void __stack_chk_fail_local(void) {
    __stack_chk_fail();
}

void splash_screen()
{
    printk("  *BlueOS* (kernel)", cursor_y++, WHITE);
    printk("type 'help' for a help", cursor_y++, WHITE);
}



void load_and_run_program(unsigned char* program_buffer, int size) {
    printk("IPE: Verifying program integrity...", cursor_y++, WHITE);

    int status = ipe_verify_binary(program_buffer, size);

    if (status == 1) {
        printk("IPE: [OK] Valid signature. Running...", cursor_y++, GREEN);
        
    
        // ((void (*)(void))program_buffer)(); 
        
    } else {
        printk("SECURITY ALERT: Attempted to run unauthorized binary!", cursor_y++, RED);

    }
}



void k_main(void)
{
    init_all();
    init_vga(VGA_WHITE, VGA_BLACK);
    clear_screen();
    splash_screen();

    cursor_y++;
    cursor_y++;
   


    while (1) {
        keyboard_handler();
    }
}
// GUII
// init_vga_fnc();
// draw_string(50, 50, GUI_WHITE, "TEXT");



/*

void create_dosbox_ui()
{
  fill_box(0, 0, 0, BOX_MAX_WIDTH - 8, 14, VGA_BLUE);
  draw_box(BOX_DOUBLELINE, 0, 0, BOX_MAX_WIDTH - 10, 12, VGA_WHITE, VGA_BLUE);

  gotoxy(2, 1);
  print_color_string("Alert", VGA_BRIGHT_GREEN, VGA_BLUE);

  gotoxy(2, 3);
  print_color_string("TESTING XD", VGA_WHITE, VGA_BLUE);

  gotoxy(2, 4);
  print_color_string("INTRO", VGA_YELLOW, VGA_BLUE);

  gotoxy(0, 14);

}

*/
