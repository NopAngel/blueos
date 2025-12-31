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


int cursor_x = 0;
int cursor_y = 0;

void splash_screen()
{
    printk("  *BlueOS* (kernel)", cursor_y++, WHITE);
    printk("type 'help' for a help", cursor_y++, WHITE);
}


int k_main(void)
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
