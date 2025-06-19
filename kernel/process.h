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

// Process states (minimal set)
typedef enum {
    PROCESS_READY = 0,
    PROCESS_RUNNING = 1,
    PROCESS_TERMINATED = 2
} process_state_t;

// Simple CPU context (registers only, no page directory)
typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, esp, ebp;
    uint32_t eip, eflags;
} cpu_context_t;

// Minimal process structure
typedef struct process {
    int pid;                        // Process ID
    process_state_t state;          // Process state
    cpu_context_t context;          // CPU registers
    void* stack;                    // Stack pointer (allocated by kmalloc)
    struct process* next;           // Next in ready queue
    char name[32];                  // Process name
} process_t;

// Global variables
extern process_t* current_process;
extern process_t* ready_queue_head;
extern process_t* ready_queue_tail;
extern process_t process_table[MAX_PROCESSES];
extern int next_pid;

// Function declarations
void process_init(void);
int process_create(void (*entry_point)(void), const char* name);
void process_switch(void);
void process_yield(void);
void process_exit(void);
void process_list(void);

// Assembly function (to be implemented)
extern void switch_context(cpu_context_t* old_context, cpu_context_t* new_context);

#endif // PROCESS_H