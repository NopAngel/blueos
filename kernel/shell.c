#include <lib/string.h>
#include <blueos/printk.h>
#include <blueos/colors.h>
#include <version.h>
#include <auth.h>

extern int current_user_index;
extern char current_user[];

extern int cursor_x;
extern int cursor_y;




void execute_shell_command(char* input) {
    if (strlen(input) == 0) goto prompt;

    if (current_user_index == -1) {
        if (strncmp(input, "login ", 6) == 0) {
            char* name = input + 6;
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
    else if (strlen(input) > 0) {
            printk(RED, "\nPermission denied. Please login first.\n");
        }
     if (current_user_index == -1) {
            printk(WHITE, "\nblueos login: ");
        }
    } 
    else {
        if (strcmp(input, "main") == 0)  printk(GREEN, "\nTHANKS GOD FOR ALL!\n");
        else if (strcmp(input, "version") == 0) {
            printk(CYAN, "\nBlueOS Kernel v%s\n", UTS_RELEASE);
            printk(WHITE, "Arch: %s | Compiler: %s\n", BLUEOS_ARCH, COMPILER_INFO);
        }

        else if (strcmp(input, "whoami") == 0) {
            printk(CYAN, "\nYou are: %s\n", users[current_user_index].username);
        }
        else if (strncmp(input, "sysctl -a", 9) == 0) {
            sysctl_list();
        }
        else if (strcmp(input, "battery") == 0) {
            int bat = get_bat_level();
            char* status = get_bat_charging() ? "Charging" : "Discharging";
            
            printk(YELLOW, "Battery: %d%% [%s]\n", bat, status);
        }
        else if (strncmp(input, "cat ", 4) == 0) {
            cat(input + 4);
        }
        else if (strncmp(input, "sysctl -w ", 10) == 0) {
            char *cmd = input + 10;
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
        else if (strcmp(input, "logout") == 0) {
            current_user_index = -1;
            printk(YELLOW, "\nLogged out. Session closed.\n");
            printk(WHITE, "blueos login: ");
        }
        else if (strncmp(input, "cd ", 3) == 0) {
            cd(input + 3);
        }
        else if (strcmp(input, "help") == 0) {
            vfs_cat("/base/inf/info.bluehelp");
        }
        else if (strncmp(input, "rm ", 3) == 0) {
            fs_rm(input + 3);
        }
        else if (strncmp(input, "rmdir ", 6) == 0) {
            fs_rmdir(input + 6);
        }
        else if (strcmp(input, "clear") == 0) {
            clear_screen();
            cursor_y = 0;
            cursor_x = 0; 
        }
        else if (strcmp(input, "pwd") == 0) {
            pwd();
        }
        else if (strcmp(input, "ls") == 0) {
            printk(WHITE, "\n"); 
            list_items();
     
        }
        else if (strcmp(input, "vfs-ls") == 0) {
            printk(WHITE, "\n"); 
            vfs_ls();
     
        }
        else if (strncmp(input, "cat ", 8) == 0) {
            printk(WHITE, "\n"); 
            vfs_cat(input + 8);
     
        }
        else if (strncmp(input, "mkdir ", 6) == 0) {
            mkdir(input + 6);
        }

        else if (strncmp(input, "jfs-mkdir ", 10) == 0) {
            jfs_mkdir(input + 10);
        }
        else if (strcmp(input, "jfs-ls") == 0) {
            printk(WHITE, "\n"); 
            jfs_ls();
        }
        else if (strncmp(input, "jfs-touch ", 10) == 0) {
            jfs_touch(input + 10);
        }
        else if (strncmp(input, "touch ", 6) == 0) {
            touch(input + 6, "");
            printk(GREEN, "\nFile created.\n");
        }
        else if (strcmp(input, "xfs-ls") == 0)
        {
           xfs_ls(); 
        }
        else if (strncmp(input, "xfs-createnode ", 10) == 0)
        {
            xfs_create(input + 10);
        }
        else if (strcmp(input, "bluefetch") == 0) {
            print_raccoon_real(); 
        }
        else if (strcmp(input, "reboot") == 0 || strcmp(input, "restart") == 0) {
            sys_reboot(); 
        }
        else if (strcmp(input, "poweroff") == 0 || strcmp(input, "shutdown") == 0) {
            sys_shutdown(); 
        }
        else if (strlen(input) > 0) {
            printk(RED, "\nERR: Command not found\n");
        }
    }

prompt:
    if (current_user_index != -1) {
        printk(GREEN, "user@blueos");
        printk(WHITE, ":");
        printk(CYAN, "%s", users[current_user_index].cwd); 
        printk(WHITE, "$ ");
    } else {
        printk(WHITE, "blueos login: ");
    }
}