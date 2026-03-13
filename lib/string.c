int strlen(const char *str)
{
    int length = 0;
    while (*str != '\0') {
        length++;
        str++;
    }
    return length;
}

int strncmp(const char *str1, const char *str2, unsigned int n) {
    while (n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0) {
        return 0; 
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}

void* memcpy(void* dest, const void* src, int n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (int i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

void* memset(void* dest, int c, int n) {
    unsigned char* p = (unsigned char*)dest;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return dest;
}

int memcmp(const void* s1, const void* s2, int n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    for (int i = 0; i < n; i++) {
        if (p1[i] != p2[i]) return p1[i] - p2[i];
    }
    return 0;
}

int bcmp(const void* s1, const void* s2, int n) {
    return memcmp(s1, s2, n);
}

char *strcpy(char *dest, const char *src) 
{
    char *dest_start = dest;
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0'; 
    return dest_start;
}


char *strchr(const char *s, int c) {
    while (*s != (char)c) {
        if (!*s) return 0;
        s++;
    }
    return (char *)s;
}




char *strcat(char *dest, const char *src) {
    char *ptr = dest;

    /* Move ptr to the end of dest string */
    while (*ptr != '\0') {
        ptr++;
    }

    /* Copy src to the end of dest */
    while (*src != '\0') {
        *ptr++ = *src++;
    }

    /* Terminate with null character */
    *ptr = '\0';
    return dest;
}


char *strrchr(const char *s, int c) {
    char *last = 0;
    do {
        if (*s == (char)c) last = (char *)s;
    } while (*s++);
    return last;
}


char *strncpy(char *dest, const char *src, int n) {
    int i;

    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    /* Fill the rest of the buffer with null bytes if src is shorter than n */
    for (; i < n; i++) {
        dest[i] = '\0';
    }

    return dest;
}

char *strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    for (; *haystack; haystack++) {
        if (*haystack == *needle) {
            const char *h = haystack, *n = needle;
            while (*h && *n && *h == *n) {
                h++; n++;
            }
            if (!*n) return (char *)haystack;
        }
    }
    return 0;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int atoi(const char *s) {
    int res = 0;
    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }
    return res;
}

void itoa(int n, char* s) {
    int i = 0;
    s[i++] = n + '0';
    s[i] = '\0';
}

long simple_strtol(const char *cp, char **endp, unsigned int base) {
    long result = 0;
    
    if (!base) {
        if (*cp == '0') {
            base = 8;
            cp++;
            if ((*cp == 'x' || *cp == 'X')) {
                base = 16;
                cp++;
            }
        } else {
            base = 10;
        }
    }

    while (1) {
        unsigned int value;
        if (*cp >= '0' && *cp <= '9') value = *cp - '0';
        else if (*cp >= 'a' && *cp <= 'f') value = *cp - 'a' + 10;
        else if (*cp >= 'A' && *cp <= 'F') value = *cp - 'A' + 10;
        else break;

        if (value >= base) break;
        result = result * base + value;
        cp++;
    }

    if (endp) *endp = (char *)cp;
    return result;
}

