#include "string.h"

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

void* memset(void* dest, int val, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    unsigned char value = (unsigned char)val;
    
    for (size_t i = 0; i < n; i++) {
        d[i] = value;
    }
    
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d < s) {
        // Copy forward
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else {
        // Copy backward to handle overlap
        for (size_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    
    return 0;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++) != '\0');
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    
    return dest;
}

char* strcat(char* dest, const char* src) {
    char* d = dest;
    
    // Find end of dest
    while (*d != '\0') {
        d++;
    }
    
    // Copy src to end of dest
    while ((*d++ = *src++) != '\0');
    
    return dest;
}

char* strncat(char* dest, const char* src, size_t n) {
    char* d = dest;
    
    // Find end of dest
    while (*d != '\0') {
        d++;
    }
    
    // Copy up to n characters from src
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        d[i] = src[i];
    }
    d[i] = '\0';
    
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

char* strchr(const char* str, int c) {
    while (*str != '\0') {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }
    return (*str == (char)c) ? (char*)str : NULL;
}

char* strrchr(const char* str, int c) {
    const char* last = NULL;
    
    while (*str != '\0') {
        if (*str == (char)c) {
            last = str;
        }
        str++;
    }
    
    return (char*)last;
}

char* strstr(const char* haystack, const char* needle) {
    if (*needle == '\0') {
        return (char*)haystack;
    }
    
    while (*haystack != '\0') {
        const char* h = haystack;
        const char* n = needle;
        
        while (*h != '\0' && *n != '\0' && *h == *n) {
            h++;
            n++;
        }
        
        if (*n == '\0') {
            return (char*)haystack;
        }
        
        haystack++;
    }
    
    return NULL;
}