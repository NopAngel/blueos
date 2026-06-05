#include <kernel/printk.h>
#include <kernel/colors.h>
#include <kernel/syscall.h>
#include <drivers/vt100.h>
#include <drivers/keyboard.h>
#include <fs/fs.h>
#include <fs/vfs.h>
#include <kernel/task.h>

typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

// This is defined in kernel/printk.c, but we should probably move it to a proper header.
extern void clear_screen(void);

/* Externs de teclado y FS */
extern char get_char();
extern task_t* current_task;

/* Esta estructura debe coincidir con el pusha de tu interrupt_entry */
typedef struct {
    uint32_t ds, es, fs, gs;
    uint32_t edi, esi, ebp, esp_at_pusha, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

void syscall_handler(registers_t regs) {
    // asm volatile("sti");

    switch (regs.eax) {
        case SYS_READ: // SYS_READ (POSIX)
            // ebx: fd, ecx: buf, edx: count
            if (current_task->fds[regs.ebx] == 0) { // Check if FD maps to stdin
                if (regs.ecx == 0) return;
                char* b = (char*)regs.ecx;
                for(uint32_t i = 0; i < regs.edx; i++) {
                    b[i] = get_char();
                    if (b[i] == '\n') break;
                }
            }
            break;

        case SYS_WRITE: // SYS_WRITE (POSIX)
            // ebx: fd, ecx: buf, edx: count
            if (current_task->fds[regs.ebx] == 1 || current_task->fds[regs.ebx] == 2) {
                char* b = (char*)regs.ecx;
                for(uint32_t i = 0; i < regs.edx; i++) {
                    vt100_putc(b[i]);
                }
            }
            break;

        case SYS_OPEN: // SYS_OPEN (POSIX)
            // ebx: path
            regs.eax = (uint32_t)vfs_findfile((const char*)regs.ebx);
            break;

        case SYS_PRINTK:
            // Handles printing a string to the console.
            // ebx: string pointer
            if (regs.ebx != 0) {
                printk((const char*)regs.ebx);
            }
            break;

        case SYS_CLEAR:
            // Clears the screen.
            clear_screen();
            break;

        case SYS_EXIT:
            // For now, just print an exit message.
            // In the future, this will terminate the current process.
            printk("\033[33mUserspace program exited with code %d. Halting system.\n\033[0m", regs.ebx);

            asm volatile("cli");
            for(;;) { asm volatile("hlt"); }
            break;

        default:
            printk("\033[31mUnknown Syscall: %d\033[0m\n", regs.eax);
            break;
    }

    // asm volatile("cli");
}
