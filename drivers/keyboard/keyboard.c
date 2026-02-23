#include <drivers/keyboard.h>
#include <drivers/k_language.h>
#include <blueos/printk.h>
#include <blueos/colors.h>
#include <lib/string.h>
#include <blueos/ports.h>
#include <fs/fs.h>
#include <auth.h>
#include <idt.h>
#include <version.h>
#include <fs/vfs.h>

#define KEYBOARD_PORT 0x60
#define SCREEN_BUFFER ((unsigned char *)0xb8000)
#define SCREEN_COLUMNS 80
#define SCREEN_ROWS 25
#define INPUT_BUFFER_SIZE 255


void update_cursor(int x, int y) {
    uint16_t pos = y * SCREEN_COLUMNS + x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

extern char current_user[32];
extern void print_raccoon_real(void);
extern int current_user_index; 
extern void start_nano(const char* filename);

char input_buffer[INPUT_BUFFER_SIZE];
int input_index = 0;
int caps_lock = 0;
int shift_pressed = 0;
volatile unsigned char last_scancode = 0;
volatile int ctrl_pressed = 0;

int cursor_y = 0;
int cursor_x = 0;



void scroll_screen() {
    for (int i = 1; i < SCREEN_ROWS; i++) {
        for (int j = 0; j < SCREEN_COLUMNS * 2; j++) {
            SCREEN_BUFFER[(i - 1) * SCREEN_COLUMNS * 2 + j] = SCREEN_BUFFER[i * SCREEN_COLUMNS * 2 + j];
        }
    }
    for (int j = 0; j < SCREEN_COLUMNS; j++) {
        int pos = ((SCREEN_ROWS - 1) * SCREEN_COLUMNS + j) * 2;
        SCREEN_BUFFER[pos] = ' ';      
        SCREEN_BUFFER[pos + 1] = 0x07;  
    }
    cursor_y = SCREEN_ROWS - 1; 
}

void handle_backspace() {
    if (input_index > 0) {
        input_index--;
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = SCREEN_COLUMNS - 1;
        }
        
        int pos = (cursor_y * SCREEN_COLUMNS + cursor_x) * 2;
        SCREEN_BUFFER[pos] = ' ';      
        SCREEN_BUFFER[pos + 1] = 0x07; 
        update_cursor(cursor_x, cursor_y);
    }
}

void put_char(char c, unsigned int color) {
    if (cursor_y >= SCREEN_ROWS) scroll_screen();

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else {
        int pos = (cursor_y * SCREEN_COLUMNS + cursor_x) * 2;
        SCREEN_BUFFER[pos] = c;
        SCREEN_BUFFER[pos + 1] = (unsigned char)color;
        cursor_x++;

        if (cursor_x >= SCREEN_COLUMNS) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    if (cursor_y >= SCREEN_ROWS) scroll_screen();
    update_cursor(cursor_x, cursor_y); 
}


int process_input() {
    input_buffer[input_index] = '\0'; 
 
    if (current_user_index == -1) {
        if (strncmp(input_buffer, "login ", 6) == 0) {
            char* name = input_buffer + 6;
            char* pass = strchr(name, ' '); 
            if (pass) {
                *pass = '\0';
                pass++;
                if (check_login(name, pass)) {
                    strncpy(current_user, name, 31);
                    current_user[31] = '\0';
                    clear_screen();
                    cursor_y = 0; cursor_x = 0; 
                    printk(GREEN, "Welcome to BlueOS. System ready.\n");
                } else {
                    printk(RED, "\nLogin Failed!\n");
                }
            }
        } 
        else if (strlen(input_buffer) > 0) {
            printk(RED, "\nPermission denied. Please login first.\n");
        }
        
        if (current_user_index == -1) {
            printk(WHITE, "\nblueos login: ");
        }
    } 
    else {

        if (strcmp(input_buffer, "main") == 0) {
            printk(GREEN, "\nTHANKS GOD FOR ALL!\n");
        }
        else if (strcmp(input_buffer, "version") == 0) {
            printk(CYAN, "\nBlueOS Kernel v%s\n", UTS_RELEASE);
            printk(WHITE, "Arch: %s | Compiler: %s\n", BLUEOS_ARCH, COMPILER_INFO);
        }
        else if (strncmp(input_buffer, "nano ", 5) == 0) {
            start_nano(input_buffer + 5);
        }
        else if (strcmp(input_buffer, "whoami") == 0) {
            printk(CYAN, "\nYou are: %s\n", users[current_user_index].username);
        }
        else if (strncmp(input_buffer, "sysctl -a", 9) == 0) {
            sysctl_list();
        }
        else if (strncmp(input_buffer, "cat ", 4) == 0) {
            cat(input_buffer + 4);
        }
        else if (strncmp(input_buffer, "sysctl -w ", 10) == 0) {
            char *cmd = input_buffer + 10;
            char *name = cmd;
            char *value = strchr(cmd, '=');

            if (value) {
                *value = '\0'; 
                value++;       
                if (sysctl_set(name, value) == 0) {
                    printk(GREEN, "Variable updated.\n");
                } else {
                    printk(RED, "Error updating variable.\n");
                }
            }
        }
        else if (strcmp(input_buffer, "logout") == 0) {
            current_user_index = -1;
            printk(YELLOW, "\nLogged out. Session closed.\n");
            printk(WHITE, "blueos login: ");
        }
        else if (strncmp(input_buffer, "cd ", 3) == 0) {
            cd(input_buffer + 3);
        }
        else if (strcmp(input_buffer, "help") == 0) {
            vfs_cat("/base/inf/info.bluehelp");
        }
        else if (strncmp(input_buffer, "rm ", 3) == 0) {
            fs_rm(input_buffer + 3);
        }
        else if (strncmp(input_buffer, "rmdir ", 6) == 0) {
            fs_rmdir(input_buffer + 6);
        }
        else if (strcmp(input_buffer, "clear") == 0) {
            clear_screen();
            cursor_y = 0;
            cursor_x = 0; 
        }
        else if (strcmp(input_buffer, "pwd") == 0) {
            pwd();
        }
        else if (strcmp(input_buffer, "ls") == 0) {
            printk(WHITE, "\n"); 
            list_items();
     
        }
        else if (strcmp(input_buffer, "vfs-ls") == 0) {
            printk(WHITE, "\n"); 
            vfs_ls();
     
        }
        else if (strncmp(input_buffer, "cat ", 8) == 0) {
            printk(WHITE, "\n"); 
            vfs_cat(input_buffer + 8);
     
        }
        else if (strncmp(input_buffer, "mkdir ", 6) == 0) {
            mkdir(input_buffer + 6);
        }

        else if (strncmp(input_buffer, "jfs-mkdir ", 10) == 0) {
            jfs_mkdir(input_buffer + 10);
        }
        else if (strcmp(input_buffer, "jfs-ls") == 0) {
            printk(WHITE, "\n"); 
            jfs_ls();
        }
        else if (strncmp(input_buffer, "jfs-touch ", 10) == 0) {
            jfs_touch(input_buffer + 10);
        }
        else if (strncmp(input_buffer, "touch ", 6) == 0) {
            touch(input_buffer + 6, "");
            printk(GREEN, "\nFile created.\n");
        }
        else if (strcmp(input_buffer, "xfs-ls") == 0)
        {
           xfs_ls(); 
        }
        else if (strncmp(input_buffer, "xfs-createnode ", 10) == 0)
        {
            xfs_create(input_buffer + 10);
        }
        else if (strcmp(input_buffer, "bluefetch") == 0) {
            print_raccoon_real(); 
        }
        else if (strcmp(input_buffer, "reboot") == 0 || strcmp(input_buffer, "restart") == 0) {
            sys_reboot(); 
        }
        else if (strcmp(input_buffer, "poweroff") == 0 || strcmp(input_buffer, "shutdown") == 0) {
            sys_shutdown(); 
        }
        else if (strlen(input_buffer) > 0) {
            printk(RED, "\nERR: Command not found\n");
        }
        
        if (current_user_index != -1) {
            put_char('\n', 0x07);
            printk(GREEN, "user@blueos");
            printk(WHITE, ":");
            printk(CYAN, "%s", users[current_user_index].cwd); 
            printk(WHITE, "$ ");
        }
    }

    input_index = 0;
    update_cursor(cursor_x, cursor_y); 
    return 0;
}



unsigned char read_scancode() {
    unsigned char scancode;
    __asm__ volatile ("inb %1, %0" : "=a"(scancode) : "Nd"(KEYBOARD_PORT));
    return scancode;
}


void keyboard_handler(struct registers *r) {
    unsigned char scancode = read_scancode();
    pic_send_eoi(1); 

    if (scancode & 0x80) {
        unsigned char released = scancode & 0x7F; 
        if (released == 0x1D) ctrl_pressed = 0;
        if (released == 0x2A || released == 0x36) shift_pressed = 0;
        
        last_scancode = 0;  
        return;
    }


    if (scancode == last_scancode) {
        return;
    }

    last_scancode = scancode;

    switch (scancode) {
        case 0x1D: ctrl_pressed = 1; return;
        case 0x2A: case 0x36: shift_pressed = 1; return;
        case 0x3A: caps_lock = !caps_lock; return;
    }

   
    if (scancode < 128) {
        char ascii = scancode_to_ascii[scancode];
        if (ascii == 0) return;

        if ((caps_lock || shift_pressed) && ascii >= 'a' && ascii <= 'z') ascii -= 32;

        // Ctrl + L
        if (ctrl_pressed && ascii == 'l') {
            clear_screen();
            cursor_x = 0; cursor_y = 0;
            input_index = 0;
            update_cursor(0, 0);
            return;
        }

        if (ascii == '\b') {
            handle_backspace();
        } else if (ascii == '\n') {
            put_char('\n', WHITE);
            process_input(); 
        } else {
            if (input_index < INPUT_BUFFER_SIZE - 1) {
                input_buffer[input_index++] = ascii;
                put_char(ascii, WHITE); 
            }
        }
    }
}