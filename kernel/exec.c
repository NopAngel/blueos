#include <fs/vfs.h>
#include <kernel/malloc.h>
#include <kernel/printk.h>
#include <lib/string.h>

extern void *kmalloc(uint32_t size);

typedef struct {
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
} shell_context_t;

static int shell_context_saved = 0; 
static shell_context_t shell_saved_context;

extern void _run_user(void *entry, int argc, char **argv);


void return_to_shell(void) {
    if (!shell_context_saved) {
        for(;;);
    }

    __asm__ volatile (
        "movl %0, %%esp\n"
        "movl %1, %%ebp\n"
        "jmp *%2\n"
        :
        : "m"(shell_saved_context.esp),
          "m"(shell_saved_context.ebp),
          "m"(shell_saved_context.eip)
    );
}

void sys_exec(const char *path, int argc, char **argv) {
    vfs_node_t *bin_node = vfs_lookup(path);
    if (!bin_node) {
        printk("err: command '%s' not found.\n", path);
        return;
    }

    if (bin_node->size == 0) {
        printk("err: binary '%s' is empty.\n", path);
        return;
    }

    // FIXED: Volvemos a la dirección plana donde tu 'user.ld' espera que vivan los strings.
    // Esto es crucial para binarios crudos sin memoria virtual (Paginación).
    void *exec_pool = (void *)0x00500000; 
    uint32_t pool_size = bin_node->size + 4096; 
    
    // Limpiamos la zona y copiamos el ejecutable desde el RAMDisk
    memset(exec_pool, 0, pool_size);
    int read_bytes = vfs_read(bin_node, (char *)exec_pool, bin_node->size, 0);
    
    if (read_bytes <= 0) {
        printk("Error reading binary from storage mapping.\n");
        return;
    }

    // Configuramos los argumentos de forma segura
    char **user_argv = (char **)((uintptr_t)exec_pool + bin_node->size + 1024);
    char *arg_strings_dest = (char *)((uintptr_t)user_argv + (argc * sizeof(char *)));

    for (int i = 0; i < argc; i++) {
        if (argv[i] != NULL) {
            strcpy(arg_strings_dest, argv[i]);
            user_argv[i] = arg_strings_dest;
            arg_strings_dest += strlen(argv[i]) + 1;
        } else {
            user_argv[i] = NULL;
        }
    }

    // Guardar contexto de la shell y saltar a user space
    __asm__ volatile (
        "movl %%esp, %0\n"
        "movl %%ebp, %1\n"
        "movl $1f, %2\n"
        : "=m"(shell_saved_context.esp),
          "=m"(shell_saved_context.ebp),
          "=m"(shell_saved_context.eip)
    );
    shell_context_saved = 1;

    _run_user(exec_pool, argc, user_argv);

__asm__ volatile("1:");

    // NOTA: Como usamos memoria física estática (0x00500000), no hacemos kfree().
    // El siguiente programa que ejecutes simplemente sobreescribirá esta zona.
}