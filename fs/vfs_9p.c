/* fs/vfs_9p.c */

#include <stdint.h>
#include <fs/9p.h>
#include <blueos/printk.h>
#include <blueos/colors.h>

int v9p_session_init(struct v9p_session *v9ses) {
    if (!v9ses) return -1;

    // 1. Configuramos el Header apuntando al inicio del buffer de salida
    struct p9_header *h = (struct p9_header *)v9ses->out_buf;
    
    // 2. Llenamos los datos del mensaje TVERSION
    h->type = P9_TVERSION;
    h->tag = 0xFFFF; // El tag 0xFFFF es obligatorio para negociación de versión
    
    // 3. El tamaño máximo de mensaje (msize) va después del header
    // Usamos uint32_t* para escribir directamente en el buffer
    uint32_t *p_msize = (uint32_t *)(v9ses->out_buf + sizeof(struct p9_header));
    *p_msize = P9_MAX_BUF;

    // 4. Copiamos el string de la versión "9P2000" (formato: len[16] + string)
    uint16_t *s_len = (uint16_t *)(v9ses->out_buf + sizeof(struct p9_header) + 4);
    *s_len = 6; // longitud de "9P2000"
    
    char *s_ptr = (char *)(v9ses->out_buf + sizeof(struct p9_header) + 6);
    mm_memcpy(s_ptr, "9P2000", 6);

    // 5. El tamaño total del mensaje: header(7) + msize(4) + s_len(2) + "9P2000"(6) = 19 bytes
    h->size = sizeof(struct p9_header) + 4 + 2 + 6;

    pr_info("9P: Requesting session (msize=%d, ver=9P2000)\n", P9_MAX_BUF);
    
    return 0;
}

char* v9p_driver_read(const char *path) {
    // Aquí después pondremos la lógica de las VirtQueues de QEMU
    static char *msg = "9P: File content from Host Shared Folder (Stub)";
    return msg;
}