#include "../../include/gui/vga.h"
#include "../../include/gui/bitmap.h"

int cursor_x = 0;
int cursor_y = 0;


int k_main(void)
{
    init_vga_fnc();
    fill_screen(GUI_BLUE);
    fill_rect(0, 0, 400, 10, GUI_GREY);
}
// GUII
// init_vga_fnc();
// draw_string(50, 50, GUI_WHITE, "TEXT");

