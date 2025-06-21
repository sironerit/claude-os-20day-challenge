// ClaudeOS Shell - Day 11 Phase 1
// Simple command shell interface

#ifndef SHELL_H
#define SHELL_H

#include "types.h"

#include "kernel.h"

// Shell configuration
#define SHELL_BUFFER_SIZE 256
#define SHELL_MAX_ARGS 16

// Shell functions
void shell_init(void);
void shell_process_input(char c);
void shell_execute_command(const char* command);
void shell_print_prompt(void);

// Built-in commands
void cmd_help(void);
void cmd_clear(void);
void cmd_version(void);
void cmd_uptime(void);

// File system commands (Phase 2)
void cmd_ls(const char* path);
void cmd_cat(const char* filename);
void cmd_create(const char* filename);
void cmd_delete(const char* filename);
void cmd_write(const char* filename);

// System information commands
void cmd_meminfo(void);
void cmd_syscalls(void);

// Directory commands (Phase 3)
void cmd_mkdir(const char* dirname);
void cmd_rmdir(const char* dirname);
void cmd_cd(const char* dirname);

#endif // SHELL_H