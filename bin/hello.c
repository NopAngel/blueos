#include <kernel/colors.h>
#include <lib/syscall_user.h>

/**
 * Punto de entrada para el comando hello
 */
void main() {
  sys_print(LIGHT_CYAN, "Hello, BlueOS World!\n");
  sys_print(WHITE, "This message comes from a userspace syscall.\n");
  sys_exit(0);
}
