#include <include/fs/fs.h>
#include <include/fs/vfs.h>
#include <include/fs_init_base.h>

void init_all ()
{
    vfs_mkdir("/base");
    vfs_mkdir("/base/inf");

    char *help_content = 
        "--- BlueOS Help System ---\n"
        "Available commands:\n"
        " - bluefetch : Show system info and the raccoon.\n"
        " - ls        : List files in current directory.\n"
        " - cat <file>: Read file content.\n"
        " - login     : Authenticate user.\n"
        " - clear     : Wipe the terminal screen.\n"
        " - help      : Show this manual.\n"
        "--------------------------\n";

    vfs_create("/base/inf/info.bluehelp", help_content);
    sysfs_init();
    touch("help.txt", "dasa");

}
