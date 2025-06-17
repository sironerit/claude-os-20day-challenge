// ClaudeOS System Call Interface - Day 8
// System call definitions and constants

#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"

// System call numbers
#define SYS_HELLO       0  // Test system call
#define SYS_WRITE       1  // Write string to terminal
#define SYS_GETPID      2  // Get process ID
#define SYS_YIELD       3  // Yield CPU to other processes

// File system system calls (Day 9)
#define SYS_OPEN        5  // Open file
#define SYS_READ_FILE   6  // Read from file
#define SYS_WRITE_FILE  7  // Write to file
#define SYS_CLOSE       8  // Close file
#define SYS_MKDIR       9  // Create directory
#define SYS_LIST        10 // List directory
#define SYS_DELETE      11 // Delete file
#define SYS_FS_STATS    12 // File system statistics

// Maximum number of system calls
#define MAX_SYSCALLS 13

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

// File system system call implementations
int sys_open(uint32_t path_ptr, uint32_t mode, uint32_t arg3);
int sys_read_file(uint32_t fd, uint32_t buffer_ptr, uint32_t size);
int sys_write_file(uint32_t fd, uint32_t buffer_ptr, uint32_t size);
int sys_close(uint32_t fd, uint32_t arg2, uint32_t arg3);
int sys_mkdir(uint32_t path_ptr, uint32_t arg2, uint32_t arg3);
int sys_list(uint32_t path_ptr, uint32_t arg2, uint32_t arg3);
int sys_delete(uint32_t path_ptr, uint32_t arg2, uint32_t arg3);
int sys_fs_stats(uint32_t arg1, uint32_t arg2, uint32_t arg3);

// C wrapper functions for easy calling
int syscall_hello(void);
int syscall_write(const char* str);
int syscall_getpid(void);
int syscall_yield(void);

// File system wrapper functions
int syscall_open(const char* path, uint8_t mode);
int syscall_read_file(int fd, void* buffer, uint32_t size);
int syscall_write_file(int fd, const void* buffer, uint32_t size);
int syscall_close(int fd);
int syscall_mkdir(const char* path);
int syscall_list(const char* path);
int syscall_delete(const char* path);
int syscall_fs_stats(void);

// File system utility functions
int fs_is_initialized(void);

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