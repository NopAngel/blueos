#ifndef STRING_H
#define STRING_H

int strlen(const char *str);
int strncmp(const char *str1, const char *str2, unsigned int n);
void* memcpy(void* dest, const void* src, int n);
void* memset(void* dest, int c, int n);
int memcmp(const void* s1, const void* s2, int n);
int bcmp(const void* s1, const void* s2, int n);
char *strcpy(char *dest, const char *src);
int strcmp(const char *str1, const char *str2);
void itoa(int n, char* s);
long simple_strtol(const char *cp, char **endp, unsigned int base);
char *strncpy(char *dest, const char *src, int n);
char *strcat(char *dest, const char *src);
char *strstr(const char *haystack, const char *needle);
char *strchr(const char *s, int c);
char *strtok(char *str, const char *delim);
char *strrchr(const char *s, int c);

#endif
