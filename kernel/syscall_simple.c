// ClaudeOS Simple System Call Implementation - Day 8 
// Basic system call handlers without complex process management

#include "kernel.h"

// VGA helper function
static inline uint8_t vga_entry_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

// Simple string length function
static size_t simple_strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

// System call dispatch - simplified version
int syscall_dispatch(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    (void)arg2; (void)arg3; // Suppress unused parameter warnings
    switch (syscall_num) {
        case 0: // sys_hello
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            terminal_writestring("[SYSCALL] Hello from kernel! System calls working!\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return 0;
            
        case 1: // sys_write
            if (arg1 != 0) {
                const char* str = (const char*)arg1;
                terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
                terminal_writestring("[PROCESS] ");
                terminal_writestring(str);
                terminal_writestring("\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
                return simple_strlen(str);
            }
            return -1;
            
        case 2: // sys_getpid
            terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            terminal_writestring("[SYSCALL] Current PID: 1 (kernel process)\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return 1;
            
        default:
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("[SYSCALL] Invalid system call number\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return -1;
    }
}

// Test system calls function
void test_syscalls(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("Testing Basic System Calls:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    // Test hello syscall
    syscall_dispatch(0, 0, 0, 0);
    
    // Test write syscall
    syscall_dispatch(1, (uint32_t)"Hello from userspace!", 0, 0);
    
    // Test getpid syscall
    syscall_dispatch(2, 0, 0, 0);
    
    terminal_writestring("System call tests completed!\n\n");
}

// Initialize simple syscall system
void syscall_simple_init(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("Simple System Call subsystem initialized\n");
    terminal_writestring("Available syscalls: hello(0), write(1), getpid(2)\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}