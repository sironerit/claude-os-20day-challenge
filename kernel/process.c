// ClaudeOS Process Management - Day 7 Minimal Implementation
// Simple process management implementation

#include "process.h"
#include "kernel.h"
#include "heap.h"
#include "timer.h"

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

static int simple_strcmp(const char* str1, const char* str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

// Initialize process management system
void process_init(void) {
    // Clear process table (Day 15 enhanced)
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = INVALID_PID;  // Mark as unused
        process_table[i].parent_pid = INVALID_PID;
        process_table[i].state = PROCESS_TERMINATED;
        process_table[i].stack = NULL;
        process_table[i].stack_size = 0;
        process_table[i].next = NULL;
        process_table[i].creation_time = 0;
        process_table[i].cpu_time = 0;
        process_table[i].exit_code = 0;
        process_table[i].memory_usage = 0;
        // Clear name
        for (int j = 0; j < 32; j++) {
            process_table[i].name[j] = '\0';
        }
    }
    
    // Initialize queue
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    
    // Setup kernel process (Day 15 enhanced)
    current_process = &process_table[KERNEL_PID];
    current_process->pid = KERNEL_PID;
    current_process->parent_pid = INVALID_PID;
    current_process->state = PROCESS_RUNNING;
    strcpy_local(current_process->name, "kernel");
    current_process->stack = NULL;  // Kernel uses current stack
    current_process->stack_size = 0;
    current_process->next = NULL;
    current_process->creation_time = get_uptime_seconds();
    current_process->cpu_time = 0;
    current_process->exit_code = 0;
    current_process->memory_usage = 0;
    
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
    
    // Initialize process (Day 15 enhanced)
    process_t* process = &process_table[slot];
    process->pid = next_pid++;
    process->parent_pid = current_process ? current_process->pid : INVALID_PID;
    process->state = PROCESS_CREATED;
    strcpy_local(process->name, name);
    process->creation_time = get_uptime_seconds();
    process->cpu_time = 0;
    process->exit_code = 0;
    
    // Allocate stack (Day 15 enhanced)
    process->stack = kmalloc(STACK_SIZE);
    if (!process->stack) {
        terminal_writestring("[PROCESS] ERROR: Failed to allocate stack\n");
        process->pid = INVALID_PID;
        return INVALID_PID;
    }
    process->stack_size = STACK_SIZE;
    process->memory_usage = STACK_SIZE;
    
    // Initialize context (minimal)
    process->context.esp = (uint32_t)process->stack + STACK_SIZE - 4;
    process->context.ebp = process->context.esp;
    process->context.eip = (uint32_t)entry_point;
    process->context.eflags = DEFAULT_EFLAGS;
    
    // Set state to ready and add to ready queue
    process->state = PROCESS_READY;
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

// Find process by PID (Day 15)
process_t* process_find(int pid) {
    if (pid < 0 || pid >= MAX_PROCESSES) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid && process_table[i].pid != INVALID_PID) {
            return &process_table[i];
        }
    }
    return NULL;
}

// Get process state as string (Day 15)
const char* process_state_string(process_state_t state) {
    switch (state) {
        case PROCESS_READY: return "READY";
        case PROCESS_RUNNING: return "RUNNING";
        case PROCESS_BLOCKED: return "BLOCKED";
        case PROCESS_TERMINATED: return "TERMINATED";
        case PROCESS_CREATED: return "CREATED";
        default: return "UNKNOWN";
    }
}

// Enhanced process exit (Day 15)
void process_exit(int exit_code) {
    if (!current_process || current_process->pid == KERNEL_PID) {
        terminal_writestring("[PROCESS] Cannot exit kernel process\n");
        return;
    }
    
    current_process->state = PROCESS_TERMINATED;
    current_process->exit_code = exit_code;
    
    // Free stack memory
    if (current_process->stack) {
        kfree(current_process->stack);
        current_process->stack = NULL;
    }
    
    terminal_printf("[PROCESS] Process '%s' (PID: %d) exited with code %d\n", 
                   current_process->name, current_process->pid, exit_code);
    
    // Simple implementation: just mark as terminated
    // In a full OS, we would switch to another process here
}

// Kill process by PID (Day 15)
void process_kill(int pid) {
    process_t* process = process_find(pid);
    if (!process) {
        terminal_printf("[PROCESS] Process PID %d not found\n", pid);
        return;
    }
    
    if (process->pid == KERNEL_PID) {
        terminal_writestring("[PROCESS] Cannot kill kernel process\n");
        return;
    }
    
    if (process->state == PROCESS_TERMINATED) {
        terminal_printf("[PROCESS] Process PID %d already terminated\n", pid);
        return;
    }
    
    process->state = PROCESS_TERMINATED;
    process->exit_code = -1; // Killed
    
    // Free stack memory
    if (process->stack) {
        kfree(process->stack);
        process->stack = NULL;
    }
    
    terminal_printf("[PROCESS] Killed process '%s' (PID: %d)\n", process->name, pid);
}

// Count processes by state (Day 15)
int process_count_by_state(process_state_t state) {
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != INVALID_PID && process_table[i].state == state) {
            count++;
        }
    }
    return count;
}

// Show detailed process information (Day 15)
void process_show_info(int pid) {
    process_t* process = process_find(pid);
    if (!process) {
        terminal_printf("[PROCESS] Process PID %d not found\n", pid);
        return;
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("Process Information:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    terminal_printf("  PID: %d\n", process->pid);
    terminal_printf("  Parent PID: %d\n", process->parent_pid);
    terminal_printf("  Name: %s\n", process->name);
    terminal_printf("  State: %s\n", process_state_string(process->state));
    terminal_printf("  Creation Time: %d seconds\n", process->creation_time);
    terminal_printf("  CPU Time: %d ticks\n", process->cpu_time);
    terminal_printf("  Memory Usage: %d bytes\n", process->memory_usage);
    
    if (process->state == PROCESS_TERMINATED) {
        terminal_printf("  Exit Code: %d\n", process->exit_code);
    }
}

// Cleanup terminated processes (Day 15)
void process_cleanup_terminated(void) {
    int cleaned = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != INVALID_PID && 
            process_table[i].state == PROCESS_TERMINATED &&
            process_table[i].pid != KERNEL_PID) {
            
            // Mark as unused
            process_table[i].pid = INVALID_PID;
            cleaned++;
        }
    }
    
    if (cleaned > 0) {
        terminal_printf("[PROCESS] Cleaned up %d terminated processes\n", cleaned);
    } else {
        terminal_writestring("[PROCESS] No terminated processes to clean up\n");
    }
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

// Legacy function removed - replaced with enhanced process_exit(int exit_code)

// Enhanced process list (Day 15)
void process_list(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("Process List (Day 15 Enhanced)\n");
    terminal_writestring("==============================\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    terminal_writestring("PID  PPID State      Name           CPU    Memory   Time\n");
    terminal_writestring("---  ---- ---------  -------------- ------ -------- -----\n");
    
    int active_count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != INVALID_PID) {
            process_t* proc = &process_table[i];
            active_count++;
            
            // PID
            terminal_printf("%3d  ", proc->pid);
            
            // Parent PID
            if (proc->parent_pid == INVALID_PID) {
                terminal_writestring("---  ");
            } else {
                terminal_printf("%3d  ", proc->parent_pid);
            }
            
            // State (color coded)
            if (proc->state == PROCESS_RUNNING) {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            } else if (proc->state == PROCESS_TERMINATED) {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            } else {
                terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            }
            
            const char* state_str = process_state_string(proc->state);
            terminal_printf("%-9s  ", state_str);
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            
            // Name
            terminal_printf("%-14s ", proc->name);
            
            // CPU time
            terminal_printf("%6d ", proc->cpu_time);
            
            // Memory usage
            if (proc->memory_usage > 1024) {
                terminal_printf("%5dK ", proc->memory_usage / 1024);
            } else {
                terminal_printf("%6d ", proc->memory_usage);
            }
            
            // Creation time
            terminal_printf("%5ds", proc->creation_time);
            
            terminal_writestring("\n");
        }
    }
    
    terminal_writestring("\n");
    terminal_printf("Total processes: %d\n", active_count);
    terminal_printf("Running: %d, Ready: %d, Blocked: %d, Terminated: %d\n",
                   process_count_by_state(PROCESS_RUNNING),
                   process_count_by_state(PROCESS_READY),
                   process_count_by_state(PROCESS_BLOCKED),
                   process_count_by_state(PROCESS_TERMINATED));
}

// Process command handler (Day 15)
void process_command_handler(int argc, char argv[][64]) {
    if (argc < 2) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("Usage: proc <command> [args]\n");
        terminal_writestring("Commands:\n");
        terminal_writestring("  init          - Initialize process management\n");
        terminal_writestring("  list          - List all processes\n");
        terminal_writestring("  info <pid>    - Show process information\n");
        terminal_writestring("  kill <pid>    - Kill process by PID\n");
        terminal_writestring("  cleanup       - Clean up terminated processes\n");
        terminal_writestring("  stats         - Show process statistics\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        return;
    }
    
    if (simple_strcmp(argv[1], "init") == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("Initializing Process Management System...\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        
        process_init();
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_writestring("Process management initialized successfully!\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        
    } else if (simple_strcmp(argv[1], "list") == 0) {
        process_list();
        
    } else if (simple_strcmp(argv[1], "info") == 0) {
        if (argc < 3) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("Usage: proc info <pid>\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return;
        }
        
        // Simple string to number conversion
        int pid = 0;
        const char* pid_str = argv[2];
        while (*pid_str >= '0' && *pid_str <= '9') {
            pid = pid * 10 + (*pid_str - '0');
            pid_str++;
        }
        
        process_show_info(pid);
        
    } else if (simple_strcmp(argv[1], "kill") == 0) {
        if (argc < 3) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("Usage: proc kill <pid>\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return;
        }
        
        // Simple string to number conversion
        int pid = 0;
        const char* pid_str = argv[2];
        while (*pid_str >= '0' && *pid_str <= '9') {
            pid = pid * 10 + (*pid_str - '0');
            pid_str++;
        }
        
        process_kill(pid);
        
    } else if (simple_strcmp(argv[1], "cleanup") == 0) {
        process_cleanup_terminated();
        
    } else if (simple_strcmp(argv[1], "stats") == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("Process Statistics:\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        
        terminal_printf("  Total slots: %d\n", MAX_PROCESSES);
        
        int total_active = 0;
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].pid != INVALID_PID) {
                total_active++;
            }
        }
        
        terminal_printf("  Active processes: %d\n", total_active);
        terminal_printf("  Available slots: %d\n", MAX_PROCESSES - total_active);
        terminal_printf("  Running: %d\n", process_count_by_state(PROCESS_RUNNING));
        terminal_printf("  Ready: %d\n", process_count_by_state(PROCESS_READY));
        terminal_printf("  Blocked: %d\n", process_count_by_state(PROCESS_BLOCKED));
        terminal_printf("  Terminated: %d\n", process_count_by_state(PROCESS_TERMINATED));
        terminal_printf("  Next PID: %d\n", next_pid);
        
    } else {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_printf("Unknown process command: %s\n", argv[1]);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    }
}