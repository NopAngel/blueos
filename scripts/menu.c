#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_PATH "../.config"
#define VERSION_PATH "../include/version.h"

typedef struct {
    char key[64];
    char value[64];
    char label[128];
    char help[256];
    int is_menu;
    int target_state;
} MenuItem;

MenuItem main_menu[5], gen_menu[4], drv_menu[5], fs_menu[5], debug_menu[5];

int v_major = 2, v_minor = 1, v_patch = 0;
int current_state = 0; 
int button_sel = 0;

void save_version_h() {
    FILE *fp = fopen(VERSION_PATH, "w");
    if (!fp) return;
    fprintf(fp, "/* SPDX-License-Identifier: GPL-2.0 */\n");
    fprintf(fp, "#ifndef _BLUEOS_VERSION_H\n#define _BLUEOS_VERSION_H\n\n");
    fprintf(fp, "#define BLUEOS_MAJOR    %d\n", v_major);
    fprintf(fp, "#define BLUEOS_MINOR    %d\n", v_minor);
    fprintf(fp, "#define BLUEOS_PATCH    %d\n\n", v_patch);
    fprintf(fp, "#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))\n");
    fprintf(fp, "#define BLUEOS_VERSION_CODE KERNEL_VERSION(BLUEOS_MAJOR, BLUEOS_MINOR, BLUEOS_PATCH)\n\n");
    fprintf(fp, "#if defined(__x86_64__)\n    #define BLUEOS_ARCH \"x86_64\"\n#else\n    #define BLUEOS_ARCH \"x86\"\n#endif\n\n");
    fprintf(fp, "#define COMPILER_INFO \"gcc version \" __VERSION__\n");
    fprintf(fp, "#define BLUEOS_NAME \"BlueOS\"\n\n");
    fprintf(fp, "#define UTS_RELEASE    \"%d.%d.%d-blueos\"\n", v_major, v_minor, v_patch);
    fprintf(fp, "#define UTS_VERSION    \"#1 SMP PREEMPT \" __DATE__ \" \" __TIME__\n");
    fprintf(fp, "#define UTS_MACHINE    BLUEOS_ARCH\n\n");
    fprintf(fp, "static inline const char* get_kernel_banner(void) { return \"BlueOS version \" UTS_RELEASE \" (\" COMPILER_INFO \") \" UTS_VERSION; }\n\n");
    fprintf(fp, "#endif\n");
    fclose(fp);
}

void load_config_from_file() {
    FILE *fp = fopen(CONFIG_PATH, "r");
    if (!fp) return;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || !strchr(line, '=')) continue;
        
        char key[64], val[64];
        sscanf(line, "%[^=]=%s", key, val);

        MenuItem *all_menus[] = {gen_menu, drv_menu, fs_menu};
        int counts[] = {3, 4, 4};
        
        for(int m=0; m<3; m++) {
            for(int i=0; i<counts[m]; i++) {
                if (strcmp(all_menus[m][i].key, key) == 0) {
                    strcpy(all_menus[m][i].value, val);
                }
            }
        }
    }
    fclose(fp);
}

void load_version_from_h() {
    FILE *fp = fopen(VERSION_PATH, "r");
    if (!fp) return; 

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "#define BLUEOS_MAJOR")) 
            sscanf(line, "#define BLUEOS_MAJOR %d", &v_major);
        else if (strstr(line, "#define BLUEOS_MINOR")) 
            sscanf(line, "#define BLUEOS_MINOR %d", &v_minor);
        else if (strstr(line, "#define BLUEOS_PATCH")) 
            sscanf(line, "#define BLUEOS_PATCH %d", &v_patch);
    }
    fclose(fp);

    sprintf(debug_menu[0].value, "%d", v_major);
    sprintf(debug_menu[1].value, "%d", v_minor);
    sprintf(debug_menu[2].value, "%d", v_patch);
}


void save_config() {
    FILE *fp = fopen(CONFIG_PATH, "w");
    if (!fp) return;

    fprintf(fp, "# BlueOS Generated Configuration\n");

    char* get_v(const char* key) {
        MenuItem *menus[] = {gen_menu, drv_menu, fs_menu, debug_menu};
        int counts[] = {3, 4, 4, 3}; 
        for(int m=0; m<4; m++) {
            for(int i=0; i<counts[m]; i++) {
                if(strcmp(menus[m][i].key, key) == 0) return menus[m][i].value;
            }
        }
        return "n";
    }

    fprintf(fp, "CONFIG_ARCH=%s\n",         get_v("CONFIG_ARCH"));
    fprintf(fp, "CONFIG_DEBUG=%s\n",        get_v("CONFIG_DEBUG"));
    fprintf(fp, "CONFIG_OPTIMIZATION=%s\n", get_v("CONFIG_OPTIMIZATION"));
    fprintf(fp, "CONFIG_KEYBOARD=%s\n",     get_v("CONFIG_KEYBOARD"));
    fprintf(fp, "CONFIG_SPEAKER=%s\n",      get_v("CONFIG_SPEAKER"));
    fprintf(fp, "CONFIG_SOUNDBLASTER=%s\n", get_v("CONFIG_SOUNDBLASTER"));
    fprintf(fp, "CONFIG_MODULES=%s\n",      get_v("CONFIG_MODULES"));
    fprintf(fp, "CONFIG_VFS=%s\n",          get_v("CONFIG_VFS"));
    fprintf(fp, "CONFIG_SYSFS=%s\n",        get_v("CONFIG_SYSFS"));
    fprintf(fp, "CONFIG_JFS=%s\n",          get_v("CONFIG_JFS"));
    fprintf(fp, "CONFIG_XFS=%s\n",          get_v("CONFIG_XFS"));

    fclose(fp);
}

void init_data() {
    // MAIN MENU
    strcpy(main_menu[0].label, "General Setup"); main_menu[0].is_menu = 1; main_menu[0].target_state = 1;
    strcpy(main_menu[1].label, "Device Drivers"); main_menu[1].is_menu = 1; main_menu[1].target_state = 2;
    strcpy(main_menu[2].label, "File Systems"); main_menu[2].is_menu = 1; main_menu[2].target_state = 3;
    strcpy(main_menu[3].label, "Debug and Compilation"); main_menu[3].is_menu = 1; main_menu[3].target_state = 4;

    // DEBUG AND COMPILATION
    strcpy(debug_menu[0].label, "Kernel Major Version"); strcpy(debug_menu[0].value, "3"); 
    strcpy(debug_menu[1].label, "Kernel Minor Version"); strcpy(debug_menu[1].value, "1");
    strcpy(debug_menu[2].label, "Kernel Patch Level");   strcpy(debug_menu[2].value, "0");

    // GENERAL / DRIVERS / FS 
    strcpy(gen_menu[0].key, "CONFIG_ARCH"); strcpy(gen_menu[0].value, "x86"); strcpy(gen_menu[0].label, "Architecture (x86/x86_64)");
    strcpy(fs_menu[0].key, "CONFIG_VFS"); strcpy(fs_menu[0].value, "y"); strcpy(fs_menu[0].label, "VFS Support");
    strcpy(fs_menu[1].key, "CONFIG_SYSFS"); strcpy(fs_menu[1].value, "y"); strcpy(fs_menu[1].label, "SYSFS Support");
    strcpy(fs_menu[2].key, "CONFIG_JFS"); strcpy(fs_menu[2].value, "y"); strcpy(fs_menu[2].label, "JFS Support");
    strcpy(fs_menu[3].key, "CONFIG_XFS"); strcpy(fs_menu[3].value, "y"); strcpy(fs_menu[3].label, "XFS Support");


    strcpy(gen_menu[0].label, "Target Architecture");
    strcpy(gen_menu[0].key, "CONFIG_ARCH");
    strcpy(gen_menu[0].help, "Select the CPU architecture. x86 is 32-bit, x86_64 is 64-bit mode.");

    strcpy(drv_menu[0].key, "CONFIG_KEYBOARD"); strcpy(drv_menu[0].value, "y"); strcpy(drv_menu[0].label, "Keyboard Support");
    strcpy(drv_menu[1].label, "PC Speaker Support");
    strcpy(drv_menu[1].key, "CONFIG_SPEAKER");
    strcpy(drv_menu[1].help, "Enable the classic PC Beep sound using PIT Channel 2.");
}


void draw_about_blueos() {
    int w = 54, h = 12;
    int start_y = (LINES - h) / 2;
    int start_x = (COLS - w) / 2;

    attron(COLOR_PAIR(0));
    for(int i=1; i<=h; i++) mvaddch(start_y + i, start_x + w, ' ');
    for(int i=1; i<=w; i++) mvaddch(start_y + h, start_x + i, ' ');

    attron(COLOR_PAIR(2));
    for(int j=0; j<h; j++) for(int i=0; i<w; i++) mvaddch(start_y + j, start_x + i, ' ');
    box(stdscr, 0, 0);

    attron(A_BOLD | COLOR_PAIR(3));
    mvprintw(start_y + 1, (COLS - 14) / 2, " ABOUT BLUEOS ");
    attroff(A_BOLD | COLOR_PAIR(3));

    mvprintw(start_y + 3, start_x + 4, "Author:  NopAngel");
    mvprintw(start_y + 4, start_x + 4, "Version: %d.%d.%d", v_major, v_minor, v_patch);
    
    mvhline(start_y + 6, start_x + 1, ACS_HLINE, w - 2);

    attron(COLOR_PAIR(3));
    mvprintw(start_y + 7, start_x + 4, "Website:");
    attroff(COLOR_PAIR(3));
    mvprintw(start_y + 7, start_x + 14, "bluekernel.vercel.app");

    attron(COLOR_PAIR(3));
    mvprintw(start_y + 8, start_x + 4, "GitHub:");
    attroff(COLOR_PAIR(3));
    mvprintw(start_y + 8, start_x + 14, "github.com/NopAngel/blueos");

    mvprintw(start_y + h - 2, start_x + (w - 20) / 2, "< Press any key >");
    
    getch();
}

void draw_help_popup(const char *title, const char *help_text) {
    int w = 50, h = 10;
    int start_y = (LINES - h) / 2;
    int start_x = (COLS - w) / 2;

    attron(COLOR_PAIR(0));
    for(int i=1; i<=h; i++) mvaddch(start_y + i, start_x + w, ' ');
    for(int i=1; i<=w; i++) mvaddch(start_y + h, start_x + i, ' ');

    attron(COLOR_PAIR(2));
    for(int j=0; j<h; j++)
        for(int i=0; i<w; i++) mvaddch(start_y + j, start_x + i, ' ');
    
    box(stdscr, 0, 0);
    attron(A_BOLD);
    mvprintw(start_y, start_x + (w - strlen(title) - 2)/2, " %s ", title);
    attroff(A_BOLD);

    mvprintw(start_y + 2, start_x + 2, "Description:");
    mvprintw(start_y + 4, start_x + 2, "%.45s", help_text); 
    if(strlen(help_text) > 45) mvprintw(start_y + 5, start_x + 2, "%.45s", help_text + 45);

    mvprintw(start_y + h - 2, start_x + (w - 18)/2, "< Press any key >");
    
    getch(); 
}
void draw_kconfig(const char *title, MenuItem *items, int count, int sel_item) {
    int w = 76, h = 20;
    int start_y = (LINES - h) / 2;
    int start_x = (COLS - w) / 2;
    wbkgd(stdscr, COLOR_PAIR(1));
    attron(COLOR_PAIR(2));
    for(int j=0; j<h; j++) for(int i=0; i<w; i++) mvaddch(start_y + j, start_x + i, ' ');
    box(stdscr, 0, 0);
    mvprintw(start_y, start_x + (w - strlen(title))/2, " %s ", title);

    for(int i=0; i<count; i++) {
        if(i == sel_item) attron(A_REVERSE);
        if(items[i].is_menu) mvprintw(start_y + 4 + i, start_x + 5, "   %s  --->", items[i].label);
        else {
            if(current_state == 4)
                mvprintw(start_y + 4 + i, start_x + 5, " (%s) %s", items[i].value, items[i].label);
            else 
                mvprintw(start_y + 4 + i, start_x + 5, " [%c] %s", (items[i].value[0] == 'y' || !strcmp(items[i].value, "x86_64") ? '*' : ' '), items[i].label);
        }
        attroff(A_REVERSE);
    }
    const char *btns[] = {"  Select  ", "   Exit   ", "   Help   "};
    for(int i=0; i<3; i++) {
        if(i == button_sel) attron(COLOR_PAIR(4)); else attron(COLOR_PAIR(2));
        mvprintw(start_y + h - 3, start_x + 12 + (i * 18), "<%s>", btns[i]);
    }
}

int main() {
    initscr(); start_color(); noecho(); keypad(stdscr, TRUE); curs_set(0);
    init_pair(1, COLOR_WHITE, COLOR_BLUE); init_pair(2, COLOR_BLACK, COLOR_WHITE); init_pair(4, COLOR_WHITE, COLOR_BLACK);
    init_data();
    load_config_from_file();
    load_version_from_h();

    int sel = 0;
    while(1) {
        MenuItem *items; int count; char *title;
        switch(current_state) {
            case 1: items = gen_menu; count = 1; title = "General Setup"; break;
            case 2: items = drv_menu; count = 3; title = "Device Drivers"; break;
            case 3: items = fs_menu;  count = 4; title = "File Systems"; break;
            case 4: items = debug_menu; count = 3; title = "Debug and Compilation"; break;
            default: items = main_menu; count = 4; title = "Main Menu"; break;
        }

        draw_kconfig(title, items, count, sel);
        int ch = getch();

        if(ch == KEY_UP) sel = (sel > 0) ? sel - 1 : count - 1;
        else if(ch == KEY_DOWN) sel = (sel < count - 1) ? sel + 1 : 0;
        else if(ch == KEY_LEFT) button_sel = (button_sel > 0) ? button_sel - 1 : 2;
        else if(ch == KEY_RIGHT) button_sel = (button_sel < 2) ? button_sel + 1 : 0;
        else if(ch == '+' || ch == '-') {
            if(current_state == 4) {
                int val = atoi(items[sel].value);
                if(ch == '+') val++; else if(val > 0) val--;
                sprintf(items[sel].value, "%d", val);
                if(sel == 0) v_major = val; else if(sel == 1) v_minor = val; else v_patch = val;
            }
        }
        else if(ch == ' ' && current_state != 4) { 
            if(!strcmp(items[sel].key, "CONFIG_ARCH")) strcpy(items[sel].value, !strcmp(items[sel].value, "x86") ? "x86_64" : "x86");
            else strcpy(items[sel].value, items[sel].value[0] == 'y' ? "n" : "y");
        }
        else if(ch == 10) { // ENTER
            if(button_sel == 0) { // SELECT
                if(items[sel].is_menu) {
                    current_state = items[sel].target_state;
                    sel = 0; button_sel = 0;
                }
            } 
            else if(button_sel == 1) { // EXIT
                if(current_state == 0) { save_config(); save_version_h(); break; }
                else { current_state = 0; sel = 0; button_sel = 0; }
            }
            else if(button_sel == 2) { 
                if(current_state == 0 || items[sel].is_menu) {
                    draw_about_blueos();
                } else {
                    draw_help_popup(items[sel].label, items[sel].help);
                }
            }
        }
    }
    endwin();
    return 0;
}