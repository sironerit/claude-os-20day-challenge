// ClaudeOS String Utilities Implementation - Day 7

#include "string.h"

// Set memory to value
void* memset(void* dest, int value, size_t count) {
    unsigned char* ptr = (unsigned char*)dest;
    unsigned char val = (unsigned char)value;
    
    for (size_t i = 0; i < count; i++) {
        ptr[i] = val;
    }
    
    return dest;
}

// Copy memory
void* memcpy(void* dest, const void* src, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

// Compare memory
int memcmp(const void* ptr1, const void* ptr2, size_t count) {
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    
    for (size_t i = 0; i < count; i++) {
        if (p1[i] < p2[i]) return -1;
        if (p1[i] > p2[i]) return 1;
    }
    
    return 0;
}

// Get string length
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Convert integer to string (simple implementation)
static char int_buffer[32];
char* int_to_string(int value) {
    if (value == 0) {
        int_buffer[0] = '0';
        int_buffer[1] = '\0';
        return int_buffer;
    }
    
    int is_negative = 0;
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }
    
    int i = 0;
    while (value > 0) {
        int_buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    if (is_negative) {
        int_buffer[i++] = '-';
    }
    
    // Reverse string
    for (int j = 0; j < i / 2; j++) {
        char temp = int_buffer[j];
        int_buffer[j] = int_buffer[i - 1 - j];
        int_buffer[i - 1 - j] = temp;
    }
    
    int_buffer[i] = '\0';
    return int_buffer;
}

// Copy string
char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    
    return original_dest;
}

// Copy string with length limit
char* strncpy(char* dest, const char* src, size_t count) {
    char* original_dest = dest;
    size_t i;
    
    for (i = 0; i < count && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    
    // Pad with null bytes if necessary
    for (; i < count; i++) {
        dest[i] = '\0';
    }
    
    return original_dest;
}

// Compare strings
int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

// Compare strings with length limit
int strncmp(const char* str1, const char* str2, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (str1[i] != str2[i]) {
            return (unsigned char)str1[i] - (unsigned char)str2[i];
        }
        if (str1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

// Concatenate strings
char* strcat(char* dest, const char* src) {
    char* original_dest = dest;
    
    // Find end of dest
    while (*dest != '\0') {
        dest++;
    }
    
    // Copy src to end of dest
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    
    return original_dest;
}

// Find character in string
char* strchr(const char* str, int character) {
    char ch = (char)character;
    
    while (*str != '\0') {
        if (*str == ch) {
            return (char*)str;
        }
        str++;
    }
    
    // Handle case where character is null terminator
    if (ch == '\0') {
        return (char*)str;
    }
    
    return NULL;
}