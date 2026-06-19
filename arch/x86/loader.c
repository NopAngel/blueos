#include <fs/vfs.h>       /* FIX: Gives visibility to vfs_node_t and subsystems */
#include <kernel/printk.h>
#include <lib/string.h>

/* Forward external declarations for global memory managers */
extern void *kmalloc(uint32_t size);
extern void kfree(void *ptr);

/**
 * load_and_run_init - Resolves, loads, and executes an initial binary application
 * * Returns: Execution state status code (0 on success, negative on runtime fault)
 */
int load_and_run_init(void){
    /* 1. Resolve and locate the target file inside the active filesystem tree */
    vfs_node_t *init_node = vfs_lookup("/HELLO.BIN");
    if (!init_node) {
        printk("Loader error: Failed to locate initial executable binary /HELLO.BIN\n");
        return -1;
    }

    /* Prevent operations if the binary exists but is empty */
    uint32_t file_size = init_node->size;
    if (file_size == 0) {
        printk("Loader error: Target binary /HELLO.BIN is empty (0 bytes)\n");
        return -2;
    }

    /* 2. Dynamically allocate runtime memory space to house the binary executable code */
    uint8_t *file_buffer = (uint8_t *)kmalloc(file_size);
    if (!file_buffer) {
        printk("Loader error: Insufficient memory resources to stage executable allocation\n");
        return -3;
    }
    memset(file_buffer, 0, file_size);

    /* 3. Dispatch the read execution pipeline down to retrieve physical bytes */
    int bytes_read = vfs_read(init_node, (char *)file_buffer, file_size, 0);
    if (bytes_read <= 0) {
        printk("Loader error: Data stream read failure while processing /HELLO.BIN\n");
        kfree(file_buffer);
        return -4;
    }

    printk("Loader success: Loaded /HELLO.BIN (%u bytes into memory: 0x%X)\n", 
           file_size, (uintptr_t)file_buffer);

    /* 4. DEFINE EXECUTION ENTRY POINT AND JUMP TO USERSPACE */
    /* Cast the memory raw address into a standard function pointer call layout */
    void (*entry_point)(void) = (void (*)(void))file_buffer;
    
    /* Executing context jump into user binary space code */
    entry_point();

    /* Clean up memory allocation state if execution returns context back to kernel */
    kfree(file_buffer);
    return 0;
}

int x86_execute_flat_binary(const char *binary_path) {
    /* 1. Query virtual memory management records to find the target resource */
    vfs_node_t *bin_node = vfs_lookup(binary_path);
    if (!bin_node) {
        return -1; /* Resource not found */
    }

    uint32_t bin_size = bin_node->size;
    if (bin_size == 0) return -2;

    /* 2. Request isolated runtime container memory via system core allocation heaps */
    void *exec_buffer = kmalloc(bin_size);
    if (!exec_buffer) {
        printk("Execution Fault: Insufficient system memory resources.\n");
        return -3;
    }
    memset(exec_buffer, 0, bin_size);

    /* 3. Extract the machine instructions directly into the target execution runway buffer */
    int read_bytes = 0;
    if (bin_node->ops && bin_node->ops->read) {
        read_bytes = bin_node->ops->read(bin_node, (char *)exec_buffer, bin_size, 0);
    }

    if (read_bytes <= 0) {
        kfree(exec_buffer);
        return -4;
    }

    /* 4. Prepare execution entry point function pointer mapping schema */
    /* Cast raw memory block buffer starting address array directly into an executable function */
    void (*run_app)(void) = (void (*)(void))exec_buffer;

    printk("[ LOADER ] Launching User Space Binary Process: %s...\n", binary_path);
    printk("--------------------------------------------------\n");

    /* 5. JUMP: Transfer active CPU EIP instruction tracking index execution context directly into the code buffer */
    run_app();

    printk("--------------------------------------------------\n");
    printk("[ LOADER ] Binary Process executed context back to kernel cleanly.\n");

    /* 6. Recycle resources once user space app logic loops back and exits control */
    kfree(exec_buffer);
    return 0;
}