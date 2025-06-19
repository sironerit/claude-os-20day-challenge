// ClaudeOS System Call Implementation - Day 8 Minimal
// Basic system call handlers and dispatch table

#include "syscall.h"
#include "kernel.h"
#include "process.h"

// Simple string function for syscalls
static size_t syscall_strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

// VGA helper function
static inline uint8_t vga_entry_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

// System call dispatch table (minimal set)
syscall_fn_t syscall_table[MAX_SYSCALLS] = {
    sys_hello,     // SYS_HELLO (0)
    sys_write,     // SYS_WRITE (1)
    sys_getpid,    // SYS_GETPID (2)
    sys_yield      // SYS_YIELD (3)
};

// Main system call handler (called from assembly)
int syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    // Validate system call number
    if (syscall_num >= MAX_SYSCALLS) {
        terminal_printf("[SYSCALL] Invalid syscall number: %d\n", syscall_num);
        return SYSCALL_INVALID;
    }
    
    // Get function pointer from dispatch table
    syscall_fn_t syscall_fn = syscall_table[syscall_num];
    if (!syscall_fn) {
        terminal_printf("[SYSCALL] Null syscall function for number: %d\n", syscall_num);
        return SYSCALL_ERROR;
    }
    
    // Call the system call function
    return syscall_fn(arg1, arg2, arg3);
}

// System Call Implementations

// SYS_HELLO (0) - Test system call
int sys_hello(uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    (void)arg1; (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("[SYSCALL] Hello from kernel! System calls working! ✅\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    
    return SYSCALL_SUCCESS;
}

// SYS_WRITE (1) - Write string to terminal
int sys_write(uint32_t str_ptr, uint32_t arg2, uint32_t arg3) {
    (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    // Basic pointer validation
    if (str_ptr == 0) {
        terminal_writestring("[SYSCALL] Error: NULL string pointer\n");
        return SYSCALL_ERROR;
    }
    
    // For now, assume the pointer is valid (in a real OS, we'd validate the address)
    const char* str = (const char*)str_ptr;
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("[PROCESS] ");
    terminal_writestring(str);
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    
    return syscall_strlen(str); // Return number of characters written
}

// SYS_GETPID (2) - Get current process ID
int sys_getpid(uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    (void)arg1; (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    if (current_process) {
        return current_process->pid;
    }
    
    return 0; // Kernel process ID
}

// SYS_YIELD (3) - Yield CPU to other processes
int sys_yield(uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    (void)arg1; (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    terminal_printf("[SYSCALL] Process %d yielding CPU\n", 
                   current_process ? current_process->pid : 0);
    
    // Call the scheduler to switch to another process
    process_yield();
    
    return SYSCALL_SUCCESS;
}

// C Wrapper Functions for easy calling

// Inline assembly wrapper for system calls
static inline int do_syscall(int syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    int result;
    
    asm volatile (
        "int $0x80"
        : "=a" (result)
        : "a" (syscall_num), "b" (arg1), "c" (arg2), "d" (arg3)
        : "memory"
    );
    
    return result;
}

// Wrapper functions
int syscall_hello(void) {
    return do_syscall(SYS_HELLO, 0, 0, 0);
}

int syscall_write(const char* str) {
    return do_syscall(SYS_WRITE, (uint32_t)str, 0, 0);
}

int syscall_getpid(void) {
    return do_syscall(SYS_GETPID, 0, 0, 0);
}

int syscall_yield(void) {
    return do_syscall(SYS_YIELD, 0, 0, 0);
}


// Initialize system call subsystem
void syscall_init(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("[SYSCALL] System call subsystem initialized\n");
    terminal_printf("[SYSCALL] %d system calls available:\n", MAX_SYSCALLS);
    terminal_writestring("[SYSCALL]   0: sys_hello - Test system call\n");
    terminal_writestring("[SYSCALL]   1: sys_write - Write string to terminal\n");
    terminal_writestring("[SYSCALL]   2: sys_getpid - Get process ID\n");
    terminal_writestring("[SYSCALL]   3: sys_yield - Yield CPU\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}