// ClaudeOS Process Management - Day 7 Minimal Implementation
// Simple process management without complex features

#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

// Process configuration constants (no hardcoding)
#define MAX_PROCESSES 8
#define STACK_SIZE 0x1000      // 4KB stack
#define KERNEL_PID 0           // Kernel process ID
#define INVALID_PID -1         // Invalid/unused process ID
#define FIRST_USER_PID 1       // First user process ID
#define DEFAULT_EFLAGS 0x202   // Default EFLAGS (interrupts enabled)

// Process states (enhanced for Day 15)
typedef enum {
    PROCESS_READY = 0,
    PROCESS_RUNNING = 1,
    PROCESS_BLOCKED = 2,
    PROCESS_TERMINATED = 3,
    PROCESS_CREATED = 4
} process_state_t;

// Simple CPU context (registers only, no page directory)
typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, esp, ebp;
    uint32_t eip, eflags;
} cpu_context_t;

// Enhanced process structure (Day 15)
typedef struct process {
    int pid;                        // Process ID
    int parent_pid;                 // Parent process ID
    process_state_t state;          // Process state
    cpu_context_t context;          // CPU registers
    void* stack;                    // Stack pointer (allocated by kmalloc)
    size_t stack_size;              // Stack size
    struct process* next;           // Next in ready queue
    char name[32];                  // Process name
    uint32_t creation_time;         // Process creation time
    uint32_t cpu_time;              // CPU time used
    int exit_code;                  // Exit code
    uint32_t memory_usage;          // Memory usage in bytes
} process_t;

// Global variables
extern process_t* current_process;
extern process_t* ready_queue_head;
extern process_t* ready_queue_tail;
extern process_t process_table[MAX_PROCESSES];
extern int next_pid;

// Function declarations (enhanced for Day 15)
void process_init(void);
int process_create(void (*entry_point)(void), const char* name);
int process_create_simple(void (*entry_point)(void), const char* name);  // Phase 2
int process_execute_simple(int pid);  // Phase 3
int process_run_all_ready(void);  // Phase 4
void process_switch(void);
void process_yield(void);
void process_exit(int exit_code);
void process_kill(int pid);
void process_list(void);
process_t* process_find(int pid);
const char* process_state_string(process_state_t state);
void process_show_info(int pid);
int process_count_by_state(process_state_t state);
void process_cleanup_terminated(void);

// Process management commands
void process_command_handler(int argc, char argv[][64]);

// Assembly function (to be implemented)
extern void switch_context(cpu_context_t* old_context, cpu_context_t* new_context);

#endif // PROCESS_H