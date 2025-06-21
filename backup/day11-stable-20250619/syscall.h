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

// Day 9: File system system calls
#define SYS_OPEN   4  // Open file
#define SYS_CLOSE  5  // Close file
#define SYS_READ   6  // Read from file
#define SYS_WRITE_FILE 7  // Write to file
#define SYS_LIST   8  // List files

// Maximum number of system calls (Day 9 expanded)
#define MAX_SYSCALLS 9

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

// Day 9: File system system call implementations
int sys_open(uint32_t filename_ptr, uint32_t mode, uint32_t arg3);
int sys_close(uint32_t fd, uint32_t arg2, uint32_t arg3);
int sys_read(uint32_t fd, uint32_t buffer_ptr, uint32_t count);
int sys_write_file(uint32_t fd, uint32_t buffer_ptr, uint32_t count);
int sys_list(uint32_t arg1, uint32_t arg2, uint32_t arg3);

// C wrapper functions
int syscall_hello(void);
int syscall_write(const char* str);
int syscall_getpid(void);
int syscall_yield(void);

// Day 9: File system wrapper functions
int syscall_open(const char* filename, int mode);
int syscall_close(int fd);
int syscall_read(int fd, void* buffer, size_t count);
int syscall_write_file(int fd, const void* buffer, size_t count);
int syscall_list(void);

// System call initialization
void syscall_init(void);

// External assembly handler
extern void syscall_interrupt_handler(void);

#endif // SYSCALL_H