// ClaudeOS Process Management System - Day 7
// Process structures and definitions

#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "vmm.h"

// Process states
typedef enum {
    PROCESS_READY = 0,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

// Process priority levels
typedef enum {
    PRIORITY_HIGH = 0,
    PRIORITY_NORMAL,
    PRIORITY_LOW
} process_priority_t;

// CPU register context for context switching
typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    uint32_t eflags;
    uint32_t cr3;  // Page directory for virtual memory
} cpu_context_t;

// Process Control Block (PCB)
typedef struct process {
    uint32_t pid;                    // Process ID
    char name[32];                   // Process name
    process_state_t state;           // Process state
    process_priority_t priority;     // Process priority
    
    // CPU context for context switching
    cpu_context_t context;
    
    // Memory management
    page_directory_t* page_directory; // Virtual memory page directory
    uint32_t stack_base;            // Stack base address
    uint32_t stack_size;            // Stack size
    uint32_t heap_start;            // Heap start address
    uint32_t heap_end;              // Heap end address
    
    // Scheduling information
    uint32_t time_slice;            // Time slice for round-robin
    uint32_t cpu_time;              // Total CPU time used
    
    // Process tree
    struct process* parent;         // Parent process
    struct process* children;       // List of child processes
    struct process* next_sibling;   // Next sibling in parent's child list
    
    // Linked list for scheduler
    struct process* next;           // Next process in ready queue
    struct process* prev;           // Previous process in ready queue
} process_t;

// Process table configuration
#define MAX_PROCESSES 256
#define DEFAULT_STACK_SIZE 0x1000  // 4KB stack
#define DEFAULT_TIME_SLICE 10      // 10 timer ticks

// Global variables
extern process_t* current_process;
extern process_t* process_table[MAX_PROCESSES];
extern uint32_t next_pid;

// Process management functions
void process_init(void);
process_t* process_create(const char* name, void (*entry_point)(void), process_priority_t priority);
void process_terminate(process_t* process);
void process_exit(void);
process_t* process_get_by_pid(uint32_t pid);

// Scheduler functions
void scheduler_init(void);
void scheduler_add_process(process_t* process);
void scheduler_remove_process(process_t* process);
void schedule(void);
void yield(void);

// Context switching (implemented in assembly)
extern void context_switch(cpu_context_t* old_context, cpu_context_t* new_context);

// Utility functions
void process_print_info(process_t* process);
void process_list_all(void);

#endif // PROCESS_H