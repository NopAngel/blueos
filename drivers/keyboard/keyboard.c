#include <include/drivers/keyboard.h>
#include <include/drivers/k_language.h>
#include <include/printk.h>
#include <include/colors.h>
#include <include/lib/string.h>
#include <include/ports.h>
#include <include/fs/fs.h>
#include <include/auth.h>
#include <include/version.h>
#include <include/fs/vfs.h>

#define KEYBOARD_PORT 0x60
#define SCREEN_BUFFER ((unsigned char *)0xb8000)
#define SCREEN_COLUMNS 80
#define SCREEN_ROWS 25
#define INPUT_BUFFER_SIZE 255

extern char current_user[32];

extern void print_raccoon_real(void);
char input_buffer[INPUT_BUFFER_SIZE];
int input_index = 0;
int caps_lock = 0;
int shift_pressed = 0;
volatile unsigned char last_scancode = 0;
volatile int ctrl_pressed = 0;

int cursor_y;
int cursor_x;
extern int current_user_index; 
extern void start_nano(const char* filename);


unsigned char read_scancode() {
    unsigned char scancode;
    __asm__ volatile ("inb %1, %0" : "=a"(scancode) : "Nd"(KEYBOARD_PORT));
    return scancode;
}

/**
 * sys_cd() - Change the current working directory for the active user.
 * @path: The target directory path.
 *
 * This function updates the 'cwd' variable of the current user session.
 * It includes basic validation for absolute and relative paths.
 */
void sys_cd(const char *path) {
    if (current_user_index == -1) return;

    /* Handle "cd .." (move up one level) */
    if (strcmp(path, "..") == 0) {
        char *last_slash = strrchr(users[current_user_index].cwd, '/');
        if (last_slash && last_slash != users[current_user_index].cwd) {
            *last_slash = '\0';
        } else {
            strcpy(users[current_user_index].cwd, "/");
        }
    } 
    /* Handle absolute path */
    else if (path[0] == '/') {
        strncpy(users[current_user_index].cwd, path, 254);
    } 
    /* Handle relative path */
    else {
        if (strcmp(users[current_user_index].cwd, "/") != 0) {
            strcat(users[current_user_index].cwd, "/");
        }
        strcat(users[current_user_index].cwd, path);
    }
}

void sys_pwd() {
    if (current_user_index != -1) {
        printk(WHITE, "\n%s\n", users[current_user_index].cwd);
    }
}

void sys_ls() {
    if (current_user_index == -1) return;

    char *current_path = users[current_user_index].cwd;
    printk(WHITE, "\n");

    /* * Mock logic for VFS filtering:
     * In a real kernel, we would iterate over the dentry cache.
     * Here, we simulate listing by checking the path prefix.
     */
    vfs_list_files_in_dir(current_path);
}

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


void put_char(char c, unsigned int color) {

    if (cursor_y >= SCREEN_ROWS) {
        scroll_screen();
    }

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

 
    if (cursor_y >= SCREEN_ROWS) {
        scroll_screen();
    }
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
                    current_user[31] = '\0'; // Ensure null termination
                    clear_screen();
                    cursor_y = 0;
                    cursor_x = 0; 
                    printk(GREEN, "Welcome to BlueOS. System ready.\n");
                } else {
                    printk(RED, "\nLogin Failed!\n");
                }
            } else {
                printk(RED, "\nUsage: login <user> <pass>\n");
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
        else if (strncmp(input_buffer, "touch ", 6) == 0) {
            touch(input_buffer + 6, "");
            printk(GREEN, "\nFile created.\n");
        }
        else if (strcmp(input_buffer, "bluefetch") == 0) {
            print_raccoon_real(); 
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
    return 0;
}

void keyboard_handler() {
    unsigned char scancode = read_scancode();

    if (scancode == 0x1D) ctrl_pressed = 1;
    else if (scancode == 0x9D) ctrl_pressed = 0;

    if (ctrl_pressed && scancode == 0x26) { 
        clear_screen();
        cursor_y = 0;
        cursor_x = 0;
        if (current_user_index == -1) printk(WHITE, "blueos login: ");
        else printk(GREEN, "user@blueos:~$ ");
        return;
    }

    if (scancode == last_scancode) return;
    last_scancode = scancode;

    if (scancode & 0x80) {
        scancode &= 0x7F;
        if (scancode == 0x2A || scancode == 0x36) shift_pressed = 0;
        return;
    } else {
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
            return;
        } else if (scancode == 0x3A) {
            caps_lock = !caps_lock;
            return;
        }
    }

    if (scancode < sizeof(scancode_to_ascii)) {
        char ascii = scancode_to_ascii[scancode];

        if ((caps_lock || shift_pressed) && ascii >= 'a' && ascii <= 'z') {
            ascii -= 32;
        }

        if (ascii == '\b') {
            if (input_index > 0) {
                input_index--;
                if (cursor_x > 0) cursor_x--;
                int pos = (cursor_y * SCREEN_COLUMNS + cursor_x) * 2;
                SCREEN_BUFFER[pos] = ' ';
                SCREEN_BUFFER[pos + 1] = 0x07;
            }
        } else if (ascii == '\n') {
            process_input();
        } else if (ascii) {
            if (input_index < INPUT_BUFFER_SIZE - 1) {
                input_buffer[input_index++] = ascii;
                put_char(ascii, 0x07); 
            }
        }
    }
}