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

#endif