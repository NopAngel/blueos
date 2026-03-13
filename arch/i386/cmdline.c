#include <stdint.h>
#include <stdbool.h>

/**
 * myisspace - Basic whitespace check
 */
static inline int myisspace(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r');
}

/**
 * cmdline_find_option - Find option=argument
 * Returns length of argument, or -1 if not found.
 */
int cmdline_find_option(const char *cmdline, const char *option, char *buffer, int bufsize) {
    if (!cmdline) return -1;

    const char *ptr = cmdline;
    while (*ptr) {
        // Skip whitespace
        while (*ptr && myisspace(*ptr)) ptr++;
        if (!*ptr) break;

        // Compare option name
        const char *opt_ptr = option;
        const char *start = ptr;
        while (*ptr && *opt_ptr && *ptr == *opt_ptr) {
            ptr++;
            opt_ptr++;
        }

        // If we found the full option name followed by '='
        if (*opt_ptr == '\0' && *ptr == '=') {
            ptr++; // Skip '='
            int len = 0;
            while (*ptr && !myisspace(*ptr)) {
                if (len < bufsize - 1) {
                    buffer[len] = *ptr;
                }
                len++;
                ptr++;
            }
            if (bufsize > 0) buffer[len < bufsize ? len : bufsize - 1] = '\0';
            return len;
        }

        // Move to next word if no match
        while (*ptr && !myisspace(*ptr)) ptr++;
    }
    return -1;
}

/**
 * cmdline_find_option_bool - Find boolean flags like "quiet" or "debug"
 */
bool cmdline_find_option_bool(const char *cmdline, const char *option) {
    if (!cmdline) return false;

    const char *ptr = cmdline;
    while (*ptr) {
        while (*ptr && myisspace(*ptr)) ptr++;
        if (!*ptr) break;

        const char *opt_ptr = option;
        const char *start = ptr;
        while (*ptr && *opt_ptr && *ptr == *opt_ptr) {
            ptr++;
            opt_ptr++;
        }

        // Match if word ends here or followed by space (no '=')
        if (*opt_ptr == '\0' && (*ptr == '\0' || myisspace(*ptr))) {
            return true;
        }

        while (*ptr && !myisspace(*ptr)) ptr++;
    }
    return false;
}