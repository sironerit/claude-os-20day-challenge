#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

// VGA Color Codes - Centralized definition
typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
    VGA_COLOR_YELLOW = 14,
} vga_color;

// VGA Text Mode Functions
void terminal_initialize(void);
void terminal_writestring(const char* data);
void terminal_putchar(char c);
void terminal_setcolor(uint8_t color);
void terminal_clear(void);
void terminal_scroll(void);

// Utility Functions
size_t strlen(const char* str);

// System Functions
void kernel_panic(const char* message);

// Simple printf for debugging
void terminal_printf(const char* format, ...);

// Test process functions
void test_process_1(void);
void test_process_2(void);

#endif // KERNEL_H