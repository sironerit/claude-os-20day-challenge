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

// VGA utility functions
static inline uint8_t vga_entry_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

// Test process functions
void test_process_1(void);

// String utility functions for IPC
int strcmp(const char* str1, const char* str2);
int atoi(const char* str);

// Test process functions for multitasking
void test_process_2(void);
void test_process_ipc_sender(void);
void test_process_ipc_receiver(void);
void test_process_producer(void);
void test_process_consumer(void);
void test_process_simple(void);

#endif // KERNEL_H