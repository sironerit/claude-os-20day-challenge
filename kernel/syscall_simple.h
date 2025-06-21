// ClaudeOS Simple System Call Header - Day 8
// Simplified system call interface

#ifndef SYSCALL_SIMPLE_H
#define SYSCALL_SIMPLE_H

#include "types.h"

// System call functions
void syscall_simple_init(void);
int syscall_dispatch(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3);
void test_syscalls(void);

#endif // SYSCALL_SIMPLE_H