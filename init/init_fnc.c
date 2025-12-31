#include <include/fs/fs.h>
#include <include/fs/vfs.h>

const char* help_m = "      HELP - Manual      \n   Welcome to *BlueOS*\nThis is a kernel for servers\nCommands: \nhelp - Show this manual\nclear - clear the screen\nprint - show a message (custom, with args)\nmain - EasterEGG\npwd - Show the current path\nmkdir - Create a folder\ntouch - Create a file\ncat - Show the file contents\nls - List files and directories";
extern int cursor_y;

void init_all ()
{
    vfs_init();
    vfs_mkdir("mnt");

    mkdir("home");
    mkdir("sys");
    touch("help.txt", help_m);
    cursor_y = 0;
}
