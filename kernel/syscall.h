// ClaudeOS System Call Interface - Day 8
// System call definitions and constants

#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"

// System call numbers
#define SYS_HELLO    0  // Test system call
#define SYS_WRITE    1  // Write string to terminal
#define SYS_GETPID   2  // Get process ID
#define SYS_YIELD    3  // Yield CPU to other processes

// Maximum number of system calls
#define MAX_SYSCALLS 4

// System call return values
#define SYSCALL_SUCCESS     0
#define SYSCALL_ERROR      -1
#define SYSCALL_INVALID    -2

// System call function pointer type
typedef int (*syscall_fn_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3);

// System call dispatch table
extern syscall_fn_t syscall_table[MAX_SYSCALLS];

// System call handler (called from assembly)
int syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3);

// Individual system call implementations
int sys_hello(uint32_t arg1, uint32_t arg2, uint32_t arg3);
int sys_write(uint32_t str_ptr, uint32_t arg2, uint32_t arg3);
int sys_getpid(uint32_t arg1, uint32_t arg2, uint32_t arg3);
int sys_yield(uint32_t arg1, uint32_t arg2, uint32_t arg3);

// C wrapper functions for easy calling
int syscall_hello(void);
int syscall_write(const char* str);
int syscall_getpid(void);
int syscall_yield(void);

// Assembly system call interface
// Usage:
//   mov eax, syscall_number
//   mov ebx, arg1
//   mov ecx, arg2
//   mov edx, arg3
//   int 0x80
//   ; result in eax

// Initialize system call subsystem
void syscall_init(void);

#endif // SYSCALL_H