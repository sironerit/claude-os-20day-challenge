// ClaudeOS Process Management System - Day 7
// Process management implementation

#include "process.h"
#include "kernel.h"
#include "heap.h"
#include "vmm.h"
#include "string.h"

// Global process management variables
process_t* current_process = NULL;
process_t* process_table[MAX_PROCESSES];
uint32_t next_pid = 1;

// Scheduler variables
static process_t* ready_queue_head = NULL;
static process_t* ready_queue_tail = NULL;

// Initialize process management system
void process_init(void) {
    // Clear process table
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i] = NULL;
    }
    
    // Initialize scheduler
    scheduler_init();
    
    // Create kernel process (PID 0)
    current_process = process_create("kernel", NULL, PRIORITY_HIGH);
    current_process->pid = 0;
    current_process->state = PROCESS_RUNNING;
    
    terminal_writestring("[PROCESS] Process management system initialized\n");
    terminal_printf("[PROCESS] Kernel process created (PID: %d)\n", current_process->pid);
}

// Create a new process
process_t* process_create(const char* name, void (*entry_point)(void), process_priority_t priority) {
    // Find free slot in process table
    uint32_t pid = 0;
    for (uint32_t i = 1; i < MAX_PROCESSES; i++) {
        if (process_table[i] == NULL) {
            pid = i;
            break;
        }
    }
    
    if (pid == 0) {
        terminal_writestring("[PROCESS] ERROR: Process table full\n");
        return NULL;
    }
    
    // Allocate process control block
    process_t* process = (process_t*)kmalloc(sizeof(process_t));
    if (!process) {
        terminal_writestring("[PROCESS] ERROR: Failed to allocate PCB\n");
        return NULL;
    }
    
    // Initialize process
    process->pid = pid;
    strncpy(process->name, name, 31);
    process->name[31] = '\0';
    process->state = PROCESS_READY;
    process->priority = priority;
    
    // Initialize CPU context
    memset(&process->context, 0, sizeof(cpu_context_t));
    
    // Set up stack
    process->stack_size = DEFAULT_STACK_SIZE;
    process->stack_base = (uint32_t)kmalloc(process->stack_size);
    if (!process->stack_base) {
        terminal_writestring("[PROCESS] ERROR: Failed to allocate stack\n");
        kfree(process);
        return NULL;
    }
    
    // Initialize stack pointer (stack grows downward)
    process->context.esp = process->stack_base + process->stack_size - 4;
    process->context.ebp = process->context.esp;
    
    // Set entry point
    if (entry_point) {
        process->context.eip = (uint32_t)entry_point;
    }
    
    // Set default flags (interrupts enabled)
    process->context.eflags = 0x202;
    
    // Initialize memory management
    process->page_directory = current_page_directory;
    process->context.cr3 = (uint32_t)process->page_directory;
    process->heap_start = 0;
    process->heap_end = 0;
    
    // Initialize scheduling
    process->time_slice = DEFAULT_TIME_SLICE;
    process->cpu_time = 0;
    
    // Initialize process tree
    process->parent = current_process;
    process->children = NULL;
    process->next_sibling = NULL;
    
    // Initialize linked list pointers
    process->next = NULL;
    process->prev = NULL;
    
    // Add to process table
    process_table[pid] = process;
    
    // Add to scheduler if not kernel process
    if (entry_point) {
        scheduler_add_process(process);
    }
    
    terminal_printf("[PROCESS] Created process '%s' (PID: %d)\n", name, pid);
    return process;
}

// Terminate a process
void process_terminate(process_t* process) {
    if (!process) return;
    
    terminal_printf("[PROCESS] Terminating process '%s' (PID: %d)\n", 
                   process->name, process->pid);
    
    // Remove from scheduler
    scheduler_remove_process(process);
    
    // Free memory
    if (process->stack_base) {
        kfree((void*)process->stack_base);
    }
    
    // Remove from process table
    process_table[process->pid] = NULL;
    
    // Mark as terminated
    process->state = PROCESS_TERMINATED;
    
    // Free PCB
    kfree(process);
}

// Exit current process
void process_exit(void) {
    if (current_process && current_process->pid != 0) {
        process_terminate(current_process);
        yield(); // Switch to another process
    }
}

// Get process by PID
process_t* process_get_by_pid(uint32_t pid) {
    if (pid >= MAX_PROCESSES) return NULL;
    return process_table[pid];
}

// Initialize scheduler
void scheduler_init(void) {
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    terminal_writestring("[SCHEDULER] Round-robin scheduler initialized\n");
}

// Add process to ready queue
void scheduler_add_process(process_t* process) {
    if (!process) return;
    
    process->state = PROCESS_READY;
    process->next = NULL;
    process->prev = ready_queue_tail;
    
    if (ready_queue_tail) {
        ready_queue_tail->next = process;
    } else {
        ready_queue_head = process;
    }
    ready_queue_tail = process;
    
    terminal_printf("[SCHEDULER] Added process '%s' to ready queue\n", process->name);
}

// Remove process from ready queue
void scheduler_remove_process(process_t* process) {
    if (!process) return;
    
    // Remove from linked list
    if (process->prev) {
        process->prev->next = process->next;
    } else {
        ready_queue_head = process->next;
    }
    
    if (process->next) {
        process->next->prev = process->prev;
    } else {
        ready_queue_tail = process->prev;
    }
    
    process->next = NULL;
    process->prev = NULL;
    
    terminal_printf("[SCHEDULER] Removed process '%s' from ready queue\n", process->name);
}

// Schedule next process (Round-robin)
void schedule(void) {
    if (!ready_queue_head) {
        return; // No processes to schedule
    }
    
    // Save current process state
    if (current_process && current_process->state == PROCESS_RUNNING) {
        current_process->state = PROCESS_READY;
        if (current_process->pid != 0) { // Don't add kernel process to queue
            scheduler_add_process(current_process);
        }
    }
    
    // Get next process from ready queue
    process_t* next_process = ready_queue_head;
    scheduler_remove_process(next_process);
    
    // Switch to next process
    process_t* old_process = current_process;
    current_process = next_process;
    current_process->state = PROCESS_RUNNING;
    
    terminal_printf("[SCHEDULER] Switching from PID %d to PID %d\n", 
                   old_process ? old_process->pid : 0, current_process->pid);
    
    // Perform context switch
    if (old_process) {
        context_switch(&old_process->context, &current_process->context);
    }
}

// Yield CPU to another process
void yield(void) {
    schedule();
}

// Print process information
void process_print_info(process_t* process) {
    if (!process) return;
    
    const char* state_names[] = {"READY", "RUNNING", "BLOCKED", "TERMINATED"};
    const char* priority_names[] = {"HIGH", "NORMAL", "LOW"};
    
    terminal_printf("PID: %d | Name: %s | State: %s | Priority: %s | CPU Time: %d\n",
                   process->pid, process->name, 
                   state_names[process->state], priority_names[process->priority],
                   process->cpu_time);
}

// List all processes
void process_list_all(void) {
    terminal_writestring("\n=== Process List ===\n");
    
    uint32_t count = 0;
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i]) {
            process_print_info(process_table[i]);
            count++;
        }
    }
    
    terminal_printf("Total processes: %d\n", count);
    terminal_writestring("==================\n\n");
}