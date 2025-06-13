#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

// VGA Text Mode Functions
void terminal_initialize(void);
void terminal_writestring(const char* data);
void terminal_putchar(char c);
void terminal_setcolor(uint8_t color);

// Utility Functions
size_t strlen(const char* str);

// System Functions
void kernel_panic(const char* message);

#endif // KERNEL_H