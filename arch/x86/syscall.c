#include <kernel/printk.h>
#include <kernel/syscall.h>
#include <kernel/errno.h>
#include <kernel/malloc.h>
#include <mm/memory.h>
#include <stddef.h>
#include <lib/string.h>
#include <fs/vfs.h>

struct trap_frame {
    uint32_t gs, fs, es, ds;      
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax; 
    uint32_t error_code, interrupt_no; 
    uint32_t eip, cs, eflags, useresp, ss; 
};

typedef struct {
    vfs_node_t *node;
    uint32_t offset;
} file_desc_t;

file_desc_t process_fd_table[16] = {0};
/* Variables de estado del entorno del sistema */
char current_path[256] = "/";

/* Símbolos externos del Kernel */
extern void return_to_shell(void);
extern void *kmalloc(uint32_t size);
extern void kfree(void *ptr);
extern void sys_reboot();
/**
 * copy_from_user - Copia strings desde el espacio de usuario al kernel de forma segura
 */
static int copy_from_user(char *dest, const char *src, int max) {
    if (!src || !dest) return -1;
    int i = 0;
    // Ojo: En un futuro con paginación real, aquí validarías las tablas de páginas
    while (src[i] != '\0' && i < max - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return i;
}

/**
 * syscall_handler - Punto de entrada único para las llamadas al sistema de BlueOS
 */
void syscall_handler(struct trap_frame *regs) {
    // Por defecto, asumimos éxito (0) a menos que una syscall diga lo contrario
    int32_t syscall_return = 0; 

    switch (regs->eax) {
        
        case SYS_WRITE: { /* Caso 1: Imprimir en pantalla */
            char *user_msg = (char *)regs->ebx;
            if (user_msg == NULL) {
                syscall_return = -EINVAL;
                break;
            }
            
            // Copiamos a un buffer temporal para no leer memoria cruda de userspace
            char k_msg[512];
            copy_from_user(k_msg, user_msg, sizeof(k_msg));
            printk("%s", k_msg);
            syscall_return = 0;
            break;
        }

        case SYS_OPEN: { /* Caso 3: Abrir un archivo / dispositivo */
            char path_buf[256];
            if (copy_from_user(path_buf, (char *)regs->ebx, sizeof(path_buf)) < 0) {
                syscall_return = -EFAULT;
                break;
            }

            vfs_node_t *node = vfs_lookup(path_buf);
            if (!node) {
                syscall_return = -ENOENT;
                break;
            }

            // Buscamos un descriptor libre (nos saltamos 0, 1, 2 asignados a stdin/out/err)
            int fd = -1;
            for (int i = 3; i < 16; i++) {
                if (process_fd_table[i].node == NULL) {
                    fd = i;
                    break;
                }
            }

            if (fd == -1) {
                syscall_return = -EMFILE; // Too many open files
            } else {
                process_fd_table[fd].node = node;
                process_fd_table[fd].offset = 0;
                syscall_return = fd; // Retornamos el File Descriptor al espacio de usuario
            }
            break;
        }


        case 5: { /* Caso 5: SYS_TOUCH / Crear Archivo Vacío */
            char path_buf[256];
            if (copy_from_user(path_buf, (char *)regs->ebx, sizeof(path_buf)) < 0) {
                syscall_return = -EFAULT;
                break;
            }

            // vfs_touch devuelve 0 en éxito, o negativo en error
            syscall_return = vfs_touch(path_buf, "");
            break;
        }

        case 6: { /* Caso 6: SYS_LS / Listar Directorio */
            char user_path_buf[256];
            char full_path[256];

            if (copy_from_user(user_path_buf, (char *)regs->ebx, sizeof(user_path_buf)) < 0) {
                syscall_return = -EFAULT;
                break;
            }

            // Resolver ruta relativa o absoluta
            if (user_path_buf[0] == '/') {
                strcpy(full_path, user_path_buf);
            } else {
                sprintf(full_path, "%s%s%s", 
                        current_path, 
                        (strcmp(current_path, "/") == 0 ? "" : "/"), 
                        user_path_buf);
            }
            
            vfs_node_t *node = vfs_lookup(full_path);
            if (!node) {
                syscall_return = -ENOENT; // No such file or directory
                break;
            }

            if (node->type != VFS_TYPE_DIR) {
                syscall_return = -ENOTDIR; // Not a directory
                break;
            }

            struct vfs_dirent dirent;
            int idx = 0;
            while (vfs_readdir(node, idx, &dirent) == 0) {
                printk("%s  ", dirent.name);
                idx++;  
            }
            printk("\n");
            syscall_return = 0;
            break;
        }

        case 7: { /* Caso 7: SYS_CHDIR / Cambiar de directorio interno (Shell Built-in) */
            char user_path_buf[256];
            char absolute_target[256];

            if (copy_from_user(user_path_buf, (char *)regs->ebx, sizeof(user_path_buf)) < 0) {
                syscall_return = -EFAULT;
                break;
            }

            // Construir la ruta destino absoluta
            if (user_path_buf[0] != '/') {
                sprintf(absolute_target, "%s%s%s", 
                        strcmp(current_path, "/") == 0 ? "" : current_path, 
                        "/", user_path_buf);
            } else {
                strcpy(absolute_target, user_path_buf);
            }

            // Intentar el cambio en el VFS corporativo de BlueOS
            if (vfs_chdir(absolute_target) == 0) {
                strcpy(current_path, absolute_target);
                syscall_return = 0;
            } else {
                syscall_return = -ENOENT;
            }
            break;
        }

        case SYS_MMAP: { /* Caso 90: Mapear archivo en memoria (Xorg inyecta video aquí) */
            int fd = (int)regs->ebx;
            uintptr_t addr = (uintptr_t)regs->ecx;
            size_t length = (size_t)regs->edx;
            // Simplificamos los argumentos simulando el paso por el stack de registros
            
            if (fd < 3 || fd >= 16 || !process_fd_table[fd].node) {
                syscall_return = -EBADF;
                break;
            }

            vfs_node_t *node = process_fd_table[fd].node;
            if (node->ops && node->ops->mmap) {
                // Llama al fb_user_mmap de tu fb.c para enlazar la VRAM
                syscall_return = node->ops->mmap(node, addr, length, 0, 0, 0);
            } else {
                syscall_return = -ENODEV;
            }
            break;
        }

        case SYS_IOCTL: { /* Caso 54: Control de Dispositivo (Requerido por Xorg) */
            int fd = (int)regs->ebx;
            uint64_t request = (uint64_t)regs->ecx;
            void *user_arg = (void *)regs->edx;

            if (fd < 3 || fd >= 16 || !process_fd_table[fd].node) {
                syscall_return = -EBADF;
                break;
            }

            vfs_node_t *node = process_fd_table[fd].node;
            if (node->ops && node->ops->ioctl) {
                // Redirigimos la llamada al ioctl de fb.c que programamos antes
                syscall_return = node->ops->ioctl(node, request, user_arg);
            } else {
                syscall_return = -ENOTTY;
            }
            break;
        }

        case 8: { 
            char file_path_buf[256];
            if (copy_from_user(file_path_buf, (char *)regs->ebx, sizeof(file_path_buf)) < 0) {
                syscall_return = -EFAULT;
                break;
            }

            vfs_node_t *node = vfs_lookup(file_path_buf); 
            if (!node) {
                printk("cat: %s: No such file\n", file_path_buf);
                syscall_return = -ENOENT;
                break;
            }

            if (node->type != VFS_TYPE_FILE) {
                printk("cat: %s: Is a directory\n", file_path_buf);
                syscall_return = -EISDIR;
                break;
            }

            uint32_t size = node->size;
            if (size == 0) {
                printk("\n");
                syscall_return = 0;
                break;
            }

            char *buffer = (char *)kmalloc(size); 
            if (!buffer) {
                syscall_return = -ENOMEM;
                break;
            }

            int bytes_read = vfs_read(node, buffer, size, 0);
            if (bytes_read > 0) {
                // FIXED: Imprimimos byte por byte para que los nulos (0x00) no detengan la impresión
                // ni se filtren textos de otras partes de la RAM.
                for(int i = 0; i < bytes_read; i++) {
                    char c = buffer[i];
                    // Si es texto imprimible o salto de línea
                    if (c == '\n' || c == '\t' || (c >= 32 && c <= 126)) {
                        char tmp[2] = {c, '\0'};
                        printk(tmp);
                    } else {
                        // Si es código de máquina x86, pintamos un punto para no romper la consola
                        printk("."); 
                    }
                }
                printk("\n");
                syscall_return = 0;
            } else {
                printk("cat: error reading file\n");
                syscall_return = -EIO;
            }

            kfree(buffer); 
            break;
        }
        case 9: {
            sys_reboot();
        }

        default:
            printk("[SYSCALL] Unknown execution hook requested: %d\n", regs->eax);
            syscall_return = -ENOSYS; // Function not implemented
            break;
    }

    // Retornamos el resultado final directo a la estructura de registros del proceso
    regs->eax = syscall_return;
}