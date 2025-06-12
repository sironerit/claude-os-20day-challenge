// ClaudeOS Kernel Header
// Day 3: Basic kernel definitions and types

#ifndef KERNEL_H
#define KERNEL_H

// Standard type definitions
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned int size_t;

// Function declarations
void kernel_main(void);
void terminal_initialize(void);
void terminal_writestring(const char* data);
void terminal_setcolor(uint8_t color);
void kernel_panic(const char* message);

// System initialization
void init_descriptor_tables(void);

// Interrupt handlers
void timer_callback(void);
void keyboard_callback(void);

// Assembly helper functions (to be implemented)
extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);

#endif // KERNEL_H