// ClaudeOS Process Management - Day 7 Minimal Implementation
// Simple process management implementation

#include "process.h"
#include "kernel.h"
#include "heap.h"

// Global process management variables
process_t* current_process = NULL;
process_t* ready_queue_head = NULL;
process_t* ready_queue_tail = NULL;
process_t process_table[MAX_PROCESSES];
int next_pid = FIRST_USER_PID;

// String functions (copied from string.c for now)
static void strcpy_local(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

// Initialize process management system
void process_init(void) {
    // Clear process table
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = INVALID_PID;  // Mark as unused
        process_table[i].state = PROCESS_TERMINATED;
        process_table[i].stack = NULL;
        process_table[i].next = NULL;
    }
    
    // Initialize queue
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    
    // Setup kernel process
    current_process = &process_table[KERNEL_PID];
    current_process->pid = KERNEL_PID;
    current_process->state = PROCESS_RUNNING;
    strcpy_local(current_process->name, "kernel");
    current_process->stack = NULL;  // Kernel uses current stack
    current_process->next = NULL;
    
    terminal_writestring("[PROCESS] Minimal process system initialized\n");
    terminal_printf("[PROCESS] Kernel process ready (PID: %d)\n", current_process->pid);
}

// Create a new process (minimal implementation)
int process_create(void (*entry_point)(void), const char* name) {
    // Find free slot in process table
    int slot = INVALID_PID;
    for (int i = FIRST_USER_PID; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == INVALID_PID) {
            slot = i;
            break;
        }
    }
    
    if (slot == INVALID_PID) {
        terminal_writestring("[PROCESS] ERROR: Process table full\n");
        return INVALID_PID;
    }
    
    // Initialize process
    process_t* process = &process_table[slot];
    process->pid = next_pid++;
    process->state = PROCESS_READY;
    strcpy_local(process->name, name);
    
    // Allocate stack
    process->stack = kmalloc(STACK_SIZE);
    if (!process->stack) {
        terminal_writestring("[PROCESS] ERROR: Failed to allocate stack\n");
        process->pid = INVALID_PID;
        return INVALID_PID;
    }
    
    // Initialize context (minimal)
    process->context.esp = (uint32_t)process->stack + STACK_SIZE - 4;
    process->context.ebp = process->context.esp;
    process->context.eip = (uint32_t)entry_point;
    process->context.eflags = DEFAULT_EFLAGS;
    
    // Add to ready queue
    process->next = NULL;
    if (ready_queue_tail) {
        ready_queue_tail->next = process;
    } else {
        ready_queue_head = process;
    }
    ready_queue_tail = process;
    
    terminal_printf("[PROCESS] Created process '%s' (PID: %d)\n", name, process->pid);
    return process->pid;
}

// Simple process switch (round-robin)
void process_switch(void) {
    if (!ready_queue_head) {
        return; // No processes to switch to
    }
    
    // Get next process from queue
    process_t* next_process = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    
    if (!ready_queue_head) {
        ready_queue_tail = NULL;
    }
    
    // Add current process back to queue (if not kernel)
    if (current_process && current_process->pid != KERNEL_PID) {
        current_process->state = PROCESS_READY;
        current_process->next = NULL;
        
        if (ready_queue_tail) {
            ready_queue_tail->next = current_process;
        } else {
            ready_queue_head = current_process;
        }
        ready_queue_tail = current_process;
    }
    
    // Switch to next process
    process_t* old_process = current_process;
    current_process = next_process;
    current_process->state = PROCESS_RUNNING;
    
    terminal_printf("[PROCESS] Switch: PID %d -> PID %d\n", 
                   old_process ? old_process->pid : 0, current_process->pid);
    
    // Context switch (assembly function)
    if (old_process) {
        switch_context(&old_process->context, &current_process->context);
    }
}

// Yield CPU 
void process_yield(void) {
    process_switch();
}

// Exit current process
void process_exit(void) {
    if (current_process && current_process->pid != KERNEL_PID) {
        terminal_printf("[PROCESS] Process %d exiting\n", current_process->pid);
        
        // Free stack
        if (current_process->stack) {
            kfree(current_process->stack);
        }
        
        // Mark as terminated
        current_process->pid = INVALID_PID;
        current_process->state = PROCESS_TERMINATED;
        current_process->stack = NULL;
        
        // Switch to next process
        process_switch();
    }
}

// List all processes
void process_list(void) {
    terminal_writestring("\n=== Process List ===\n");
    
    const char* state_names[] = {"READY", "RUNNING", "TERMINATED"};
    int count = 0;
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != INVALID_PID) {
            terminal_printf("PID: %d | Name: %s | State: %s\n",
                           process_table[i].pid, 
                           process_table[i].name,
                           state_names[process_table[i].state]);
            count++;
        }
    }
    
    terminal_printf("Total processes: %d\n", count);
    terminal_writestring("==================\n\n");
}