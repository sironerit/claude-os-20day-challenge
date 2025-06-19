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

#endif // SHELL_H