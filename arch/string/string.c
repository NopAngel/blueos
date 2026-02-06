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

int strcmp(const char *str1, const char *str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char *)str1 - *(unsigned char *)str2;
}