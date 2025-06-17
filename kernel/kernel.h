#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

// Variable argument list support
typedef __builtin_va_list va_list;
#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v,l) __builtin_va_arg(v,l)

// VGA Text Mode Functions
void terminal_initialize(void);
void terminal_writestring(const char* data);
void terminal_putchar(char c);
void terminal_setcolor(uint8_t color);
void terminal_printf(const char* format, ...);

// System Functions
void kernel_panic(const char* message);

// Test process functions
void syscall_test_process(void);
void fs_test_process(void);
void test_process_1(void);
void test_process_2(void);
void idle_process(void);

#endif // KERNEL_H