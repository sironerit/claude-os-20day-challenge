// ClaudeOS System Call Interface - Day 8 Minimal Implementation
// Basic system calls without complex features

#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"

// System call numbers (no hardcoding)
#define SYS_HELLO  0  // Test system call
#define SYS_WRITE  1  // Write string to terminal
#define SYS_GETPID 2  // Get process ID
#define SYS_YIELD  3  // Yield CPU to other processes

// Maximum number of system calls (minimal set)
#define MAX_SYSCALLS 4

// System call return codes
#define SYSCALL_SUCCESS  0
#define SYSCALL_ERROR   -1
#define SYSCALL_INVALID -2

// System call function pointer type
typedef int (*syscall_fn_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3);

// System call dispatch table
extern syscall_fn_t syscall_table[MAX_SYSCALLS];

// System call handler (called from assembly)
int syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

// System call implementations
int sys_hello(uint32_t arg1, uint32_t arg2, uint32_t arg3);
int sys_write(uint32_t str_ptr, uint32_t arg2, uint32_t arg3);
int sys_getpid(uint32_t arg1, uint32_t arg2, uint32_t arg3);
int sys_yield(uint32_t arg1, uint32_t arg2, uint32_t arg3);

// C wrapper functions
int syscall_hello(void);
int syscall_write(const char* str);
int syscall_getpid(void);
int syscall_yield(void);

// System call initialization
void syscall_init(void);

// External assembly handler
extern void syscall_interrupt_handler(void);

#endif // SYSCALL_H