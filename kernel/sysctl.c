/*
 * BlueOS / kernel / sysctl.c
 * Refined Dynamic Kernel Configuration Interface
 */

#include <kernel/colors.h>
#include <kernel/printk.h>
#include <kernel/sysctl.h>
#include <lib/string.h>

// Variables globales del sistema administradas por sysctl
int kernel_debug_level = 1;
char kernel_hostname[32] = "BlueOS-Machine";
int security_pledge_log  = 1;  // Para activar/desactivar logs del motor de pledge en caliente

/* Tabla maestra de parámetros del kernel */
sysctl_entry_t sysctl_table[] = {
    {"kernel.debug",        &kernel_debug_level, SYSCTL_TYPE_INT,    1},
    {"kernel.hostname",     kernel_hostname,     SYSCTL_TYPE_STRING, 1},
    {"security.pledge_log", &security_pledge_log,SYSCTL_TYPE_INT,    1},
    {0, 0, 0, 0} /* Centinela de fin de tabla */
};

/* Función auxiliar interna: convierte string a entero de forma real (atoi) */
static int sysctl_atoi(const char *str) {
    int res = 0;
    int sign = 1;
    int i = 0;

    if (str[0] == '-') {
        sign = -1;
        i++;
    }

    for (; str[i] != '\0'; ++i) {
        if (str[i] < '0' || str[i] > '9') break;
        res = res * 10 + str[i] - '0';
    }
    return sign * res;
}

/**
 * sysctl_list: Muestra todos los parámetros disponibles con diseño limpio
 */
void sysctl_list(void) {
    printk("\n\033[35m--- BlueOS Kernel Parameters Tree ---\033[0m\n");
    for (int i = 0; sysctl_table[i].name != 0; i++) {
        printk("\033[36m  %s\033[0m = ", sysctl_table[i].name);
        
        if (sysctl_table[i].type == SYSCTL_TYPE_INT) {
            printk("%d\n", *(int *)sysctl_table[i].value);
        } else if (sysctl_table[i].type == SYSCTL_TYPE_STRING) {
            printk("\"%s\"\n", (char *)sysctl_table[i].value);
        }
    }
    printk("\n");
}

/**
 * sysctl_get: Busca un parámetro y copia su valor de forma segura a un buffer
 */
int sysctl_get(const char *name, void *out_buffer, int type_expected) {
    for (int i = 0; sysctl_table[i].name != 0; i++) {
        if (strcmp(sysctl_table[i].name, name) == 0) {
            if (sysctl_table[i].type != type_expected) return -3; // Error: Choque de tipos

            if (sysctl_table[i].type == SYSCTL_TYPE_INT) {
                *(int *)out_buffer = *(int *)sysctl_table[i].value;
            } else {
                strncpy((char *)out_buffer, (char *)sysctl_table[i].value, 31);
                ((char *)out_buffer)[31] = '\0'; // Garantizar terminación nula
            }
            return 0; // Éxito
        }
    }
    return -2; // No encontrado
}

/**
 * sysctl_set: Modifica el valor de un parámetro en tiempo de ejecución
 */
int sysctl_set(const char *name, const char *new_value) {
    if (!name || !new_value) return -4;

    for (int i = 0; sysctl_table[i].name != 0; i++) {
        if (strcmp(sysctl_table[i].name, name) == 0) {
            // Verificar permisos de escritura en la GDT de sysctl
            if (!sysctl_table[i].writable) return -1; // No escribible

            if (sysctl_table[i].type == SYSCTL_TYPE_INT) {
                /* CORREGIDO: parseo multídgito real usando sysctl_atoi */
                *(int *)sysctl_table[i].value = sysctl_atoi(new_value);
            } else if (sysctl_table[i].type == SYSCTL_TYPE_STRING) {
                /* CORREGIDO: Inyección segura previniendo desbordes en el heap/data segment */
                strncpy((char *)sysctl_table[i].value, new_value, 31);
                ((char *)sysctl_table[i].value)[31] = '\0'; // Forzar terminación nula
            }
            return 0; // Éxito
        }
    }
    return -2; // No encontrado
}