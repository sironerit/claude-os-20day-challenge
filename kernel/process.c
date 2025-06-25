// ClaudeOS Process Management - Day 7 Minimal Implementation
// Simple process management implementation

#include "process.h"
#include "kernel.h"
#include "heap.h"
#include "timer.h"
#include "vmm.h"

// Global process management variables
process_t* current_process = NULL;
process_t* ready_queue_head = NULL;
process_t* ready_queue_tail = NULL;
process_t process_table[MAX_PROCESSES];
int next_pid = FIRST_USER_PID;
static int process_system_initialized = 0;

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
    // Prevent double initialization
    if (process_system_initialized) {
        terminal_writestring("[PROCESS] System already initialized!\n");
        return;
    }
    
    // Initialize VMM and heap if not already done
    extern int heap_initialized;
    extern void heap_init(void);
    
    if (!current_page_directory) {
        terminal_writestring("[PROCESS] Initializing VMM for process management...\n");
        vmm_init();
    }
    
    if (!heap_initialized) {
        terminal_writestring("[PROCESS] Initializing heap for process management...\n");
        heap_init();
    }
    
    terminal_writestring("[PROCESS] Forcing complete process table reset...\n");
    
    // AGGRESSIVE CLEAR: Zero out entire process table first
    uint8_t* table_ptr = (uint8_t*)process_table;
    for (size_t i = 0; i < sizeof(process_table); i++) {
        table_ptr[i] = 0;
    }
    
    // Then explicitly set INVALID_PID for each slot
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = INVALID_PID;  // -1
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
    
    // Verify all slots are INVALID_PID
    terminal_printf("[PROCESS] Verifying slots (INVALID_PID = %d):\n", INVALID_PID);
    for (int i = 0; i < MAX_PROCESSES; i++) {
        terminal_printf("  Slot %d: PID=%d\n", i, process_table[i].pid);
        if (process_table[i].pid != INVALID_PID) {
            terminal_printf("[ERROR] Slot %d still has PID %d!\n", i, process_table[i].pid);
        }
    }
    
    // Initialize queue
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    
    // Setup kernel process (Day 15 enhanced) - ONLY slot 0
    terminal_writestring("[PROCESS] Setting up kernel process...\n");
    current_process = &process_table[KERNEL_PID];
    current_process->pid = KERNEL_PID;  // This is 0
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
    
    // Final verification: ALL other slots must be INVALID_PID
    terminal_writestring("[PROCESS] Final verification...\n");
    int errors = 0;
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != INVALID_PID) {
            terminal_printf("[ERROR] Slot %d has PID %d (should be %d)\n", i, process_table[i].pid, INVALID_PID);
            process_table[i].pid = INVALID_PID;  // Force correct
            errors++;
        }
    }
    
    if (errors == 0) {
        terminal_writestring("[PROCESS] ✓ All slots correctly initialized\n");
    } else {
        terminal_printf("[PROCESS] Fixed %d slot initialization errors\n", errors);
    }
    
    // Mark system as initialized
    process_system_initialized = 1;
    
    terminal_writestring("[PROCESS] ✓ Process system initialization complete\n");
    terminal_printf("[PROCESS] ✓ Kernel process ready (PID: %d)\n", current_process->pid);
    
    // Final debug count
    int debug_active = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != INVALID_PID) {
            debug_active++;
        }
    }
    terminal_printf("[DEBUG] ✓ Active processes: %d (should be 1)\n", debug_active);
}

// Phase 2: Simple process creation without stack allocation
int process_create_simple(void (*entry_point)(void), const char* name) {
    terminal_printf("[PHASE2] Starting simple process creation for '%s'\n", name);
    
    // Find free slot in process table
    int slot = INVALID_PID;
    for (int i = FIRST_USER_PID; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == INVALID_PID) {
            slot = i;
            terminal_printf("[PHASE2] Found free slot: %d\n", slot);
            break;
        }
    }
    
    if (slot == INVALID_PID) {
        terminal_writestring("[PHASE2] ERROR: Process table full\n");
        return INVALID_PID;
    }
    
    // Initialize process (Phase 2: NO STACK ALLOCATION)
    process_t* process = &process_table[slot];
    int new_pid = next_pid++;
    
    process->pid = new_pid;
    process->parent_pid = current_process ? current_process->pid : INVALID_PID;
    process->state = PROCESS_CREATED;
    strcpy_local(process->name, name);
    process->creation_time = get_uptime_seconds();
    process->cpu_time = 0;
    process->exit_code = 0;
    
    // Phase 2: NO stack allocation - use kernel stack
    process->stack = NULL;
    process->stack_size = 0;
    process->memory_usage = 0;
    
    // Minimal context (not used in Phase 2)
    process->context.esp = 0;
    process->context.ebp = 0;
    process->context.eip = (uint32_t)entry_point;
    process->context.eflags = DEFAULT_EFLAGS;
    
    // Set state to ready but DON'T add to ready queue yet
    process->state = PROCESS_READY;
    process->next = NULL;
    
    terminal_printf("[PHASE2] Created process '%s' (PID: %d) without stack\n", name, new_pid);
    return new_pid;
}

// Phase 3: Execute a ready process by PID
int process_execute_simple(int pid) {
    terminal_printf("[PHASE3] Starting execution of process PID %d\n", pid);
    
    // Find process by PID
    process_t* process = process_find(pid);
    if (!process) {
        terminal_printf("[PHASE3] ERROR: Process PID %d not found\n", pid);
        return -1;
    }
    
    // Check if process is in READY state
    if (process->state != PROCESS_READY) {
        terminal_printf("[PHASE3] ERROR: Process PID %d is not READY (state: %s)\n", 
                       pid, process_state_string(process->state));
        return -1;
    }
    
    // Get entry point from context
    void (*entry_point)(void) = (void(*)(void))process->context.eip;
    if (!entry_point) {
        terminal_printf("[PHASE3] ERROR: Process PID %d has no entry point\n", pid);
        return -1;
    }
    
    // Change state to RUNNING
    process_t* old_current = current_process;
    current_process = process;
    process->state = PROCESS_RUNNING;
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_printf("[PHASE3] Executing process '%s' (PID: %d)...\n", process->name, pid);
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    // Execute the process function directly
    entry_point();
    
    // Process completed - restore state and mark as terminated
    current_process = old_current;
    process->state = PROCESS_TERMINATED;
    process->exit_code = 0;  // Normal termination
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_printf("[PHASE3] Process '%s' (PID: %d) completed successfully\n", 
                   process->name, pid);
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    return 0;
}

// Phase 4: Execute all ready processes in sequence
int process_run_all_ready(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("[PHASE4] Starting batch execution of all ready processes\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    int executed_count = 0;
    
    // First pass: count and display ready processes
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != INVALID_PID && process_table[i].state == PROCESS_READY) {
            terminal_printf("[PHASE4] Found ready process: '%s' (PID: %d)\n", 
                           process_table[i].name, process_table[i].pid);
        }
    }
    
    // Second pass: execute all ready processes
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != INVALID_PID && process_table[i].state == PROCESS_READY) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            terminal_printf("\n[PHASE4] === Executing process %d/%d ===\n", 
                           executed_count + 1, process_count_by_state(PROCESS_READY));
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            
            int result = process_execute_simple(process_table[i].pid);
            if (result == 0) {
                executed_count++;
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                terminal_writestring("[PHASE4] Process completed successfully!\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            } else {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
                terminal_printf("[PHASE4] Process execution failed!\n");
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            }
            
            // Small delay between processes for better visibility
            for (volatile int j = 0; j < 100000; j++);
        }
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_printf("\n[PHASE4] Batch execution complete! Executed %d processes\n", executed_count);
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    return executed_count;
}

// Original process create (kept for compatibility)
int process_create(void (*entry_point)(void), const char* name) {
    terminal_printf("[DEBUG] Starting process creation for '%s'\n", name);
    terminal_printf("[DEBUG] Current next_pid: %d\n", next_pid);
    
    // Check process table state BEFORE creation
    int pre_active = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != INVALID_PID) {
            pre_active++;
            terminal_printf("[DEBUG] Pre-creation: Slot %d has PID %d\n", i, process_table[i].pid);
        }
    }
    terminal_printf("[DEBUG] Active processes before creation: %d\n", pre_active);
    
    // Find free slot in process table
    int slot = INVALID_PID;
    for (int i = FIRST_USER_PID; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == INVALID_PID) {
            slot = i;
            terminal_printf("[DEBUG] Found free slot: %d\n", slot);
            break;
        }
    }
    
    if (slot == INVALID_PID) {
        terminal_writestring("[PROCESS] ERROR: Process table full\n");
        return INVALID_PID;
    }
    
    // Initialize process (Day 15 enhanced) with EXTRA SAFETY
    process_t* process = &process_table[slot];
    terminal_printf("[DEBUG] Before setting PID, slot %d has PID %d\n", slot, process->pid);
    
    int new_pid = next_pid++;
    process->pid = new_pid;
    terminal_printf("[DEBUG] Set new PID %d to slot %d\n", new_pid, slot);
    
    process->parent_pid = current_process ? current_process->pid : INVALID_PID;
    process->state = PROCESS_CREATED;
    strcpy_local(process->name, name);
    process->creation_time = get_uptime_seconds();
    process->cpu_time = 0;
    process->exit_code = 0;
    
    terminal_printf("[DEBUG] After setting fields, process PID: %d\n", process->pid);
    
    // SKIP stack allocation for now - use static stack
    terminal_writestring("[DEBUG] Skipping stack allocation to avoid memory corruption\n");
    process->stack = NULL;  // Use kernel stack for now
    process->stack_size = 0;
    process->memory_usage = 0;
    
    // CHECK: Verify PID hasn't been corrupted
    terminal_printf("[DEBUG] After stack allocation, PID: %d\n", process->pid);
    
    // Initialize context (minimal) - use current stack
    process->context.esp = 0;  // Will use current stack
    process->context.ebp = 0;
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
    
    // Final verification
    terminal_printf("[DEBUG] Process creation complete. Final PID: %d, State: %d\n", 
                   process->pid, process->state);
    
    // Check entire table state AFTER creation
    int post_active = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != INVALID_PID) {
            post_active++;
            if (i <= 2) {  // Only show first few for brevity
                terminal_printf("[DEBUG] Post-creation: Slot %d has PID %d\n", i, process_table[i].pid);
            }
        }
    }
    terminal_printf("[DEBUG] Active processes after creation: %d\n", post_active);
    
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
    
    // Quick debug check BEFORE displaying anything
    terminal_printf("[DEBUG] Process List Check (INVALID_PID = %d):\n", INVALID_PID);
    int debug_count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != INVALID_PID) {
            debug_count++;
            terminal_printf("[DEBUG] ✓ Slot %d: PID=%d, State=%s, Name='%s'\n", 
                          i, process_table[i].pid, 
                          process_state_string(process_table[i].state),
                          process_table[i].name);
        } else {
            terminal_printf("[DEBUG] - Slot %d: INVALID (PID=%d)\n", i, process_table[i].pid);
        }
    }
    terminal_printf("[DEBUG] Found %d active processes\n\n", debug_count);
    
    if (debug_count == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("ERROR: No processes found! System may be corrupted.\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        return;
    }
    
    int active_count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != INVALID_PID) {
            process_t* proc = &process_table[i];
            active_count++;
            
            // PID
            terminal_printf("%d", proc->pid);
            if (proc->pid < 10) terminal_writestring("   ");
            else if (proc->pid < 100) terminal_writestring("  ");
            else terminal_writestring(" ");
            
            // Parent PID
            if (proc->parent_pid == INVALID_PID) {
                terminal_writestring(" ---  ");
            } else {
                terminal_writestring(" ");
                terminal_printf("%d", proc->parent_pid);
                if (proc->parent_pid < 10) terminal_writestring("   ");
                else if (proc->parent_pid < 100) terminal_writestring("  ");
                else terminal_writestring(" ");
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
            terminal_writestring(" ");
            terminal_writestring(state_str);
            // Pad state to 9 characters
            int state_len = 0;
            while (state_str[state_len]) state_len++;
            for (int pad = state_len; pad < 9; pad++) terminal_writestring(" ");
            terminal_writestring("  ");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            
            // Name
            terminal_writestring(proc->name);
            // Pad name to 14 characters
            int name_len = 0;
            while (proc->name[name_len]) name_len++;
            for (int pad = name_len; pad < 14; pad++) terminal_writestring(" ");
            terminal_writestring(" ");
            
            // CPU time
            terminal_printf("%d", proc->cpu_time);
            if (proc->cpu_time < 10) terminal_writestring("      ");
            else if (proc->cpu_time < 100) terminal_writestring("     ");
            else if (proc->cpu_time < 1000) terminal_writestring("    ");
            else if (proc->cpu_time < 10000) terminal_writestring("   ");
            else if (proc->cpu_time < 100000) terminal_writestring("  ");
            else terminal_writestring(" ");
            
            // Memory usage
            if (proc->memory_usage > 1024) {
                terminal_printf("%d", proc->memory_usage / 1024);
                terminal_writestring("K");
                int kb = proc->memory_usage / 1024;
                if (kb < 10) terminal_writestring("    ");
                else if (kb < 100) terminal_writestring("   ");
                else if (kb < 1000) terminal_writestring("  ");
                else terminal_writestring(" ");
            } else {
                terminal_printf("%d", proc->memory_usage);
                if (proc->memory_usage < 10) terminal_writestring("      ");
                else if (proc->memory_usage < 100) terminal_writestring("     ");
                else if (proc->memory_usage < 1000) terminal_writestring("    ");
                else if (proc->memory_usage < 10000) terminal_writestring("   ");
                else if (proc->memory_usage < 100000) terminal_writestring("  ");
                else terminal_writestring(" ");
            }
            
            // Creation time
            terminal_printf("%d", proc->creation_time);
            terminal_writestring("s");
            
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
        terminal_writestring("  run <name>    - Run test process directly (Phase 1)\n");
        terminal_writestring("  create2 <name> - Create process in table (Phase 2)\n");
        terminal_writestring("  execute <pid> - Execute ready process (Phase 3)\n");
        terminal_writestring("  runall       - Execute all ready processes (Phase 4)\n");
        terminal_writestring("  yield         - Yield CPU to next process\n");
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
        
    } else if (simple_strcmp(argv[1], "create") == 0) {
        if (argc < 3) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("Usage: proc create <name>\n");
            terminal_writestring("Available test processes: test1, test2\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return;
        }
        
        // External function declarations
        extern void test_process_1(void);
        extern void test_process_2(void);
        
        void (*entry_point)(void) = NULL;
        const char* proc_name = argv[2];
        
        // Select test process based on name
        if (simple_strcmp(proc_name, "test1") == 0) {
            entry_point = test_process_1;
        } else if (simple_strcmp(proc_name, "test2") == 0) {
            entry_point = test_process_2;
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_printf("Unknown test process: %s\n", proc_name);
            terminal_writestring("Available: test1, test2\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return;
        }
        
        // Create process
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_printf("Creating process '%s'...\n", proc_name);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        
        int pid = process_create(entry_point, proc_name);
        if (pid != INVALID_PID) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            terminal_printf("Process '%s' created successfully with PID %d\n", proc_name, pid);
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            
            // Start the process immediately
            process_t* process = process_find(pid);
            if (process && process->state == PROCESS_READY) {
                terminal_printf("Starting process execution...\n");
                // For now, we'll execute the process directly
                // In a full implementation, this would be handled by the scheduler
                entry_point();
                
                // After process function returns, mark as terminated
                process->state = PROCESS_TERMINATED;
                process->exit_code = 0;  // Normal termination
                
                // Free stack memory
                if (process->stack) {
                    kfree(process->stack);
                    process->stack = NULL;
                }
                
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                terminal_printf("[PROCESS] Process '%s' (PID: %d) completed and terminated\n", 
                               process->name, process->pid);
                terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            }
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("Failed to create process\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        }
        
    } else if (simple_strcmp(argv[1], "run") == 0) {
        // Phase 1: Direct execution without process creation
        if (argc < 3) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("Usage: proc run <name>\n");
            terminal_writestring("Available test processes: test1, test2\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return;
        }
        
        // External function declarations
        extern void test_process_1(void);
        extern void test_process_2(void);
        
        void (*entry_point)(void) = NULL;
        const char* proc_name = argv[2];
        
        // Select test process based on name
        if (simple_strcmp(proc_name, "test1") == 0) {
            entry_point = test_process_1;
        } else if (simple_strcmp(proc_name, "test2") == 0) {
            entry_point = test_process_2;
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_printf("Unknown test process: %s\n", proc_name);
            terminal_writestring("Available: test1, test2\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return;
        }
        
        // Phase 1: Direct execution (no process table manipulation)
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_printf("Running test process '%s' directly...\n", proc_name);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        
        // Execute the function directly
        entry_point();
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_printf("Test process '%s' completed successfully!\n", proc_name);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        
    } else if (simple_strcmp(argv[1], "create2") == 0) {
        // Phase 2: Simple process creation with process table entry
        if (argc < 3) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("Usage: proc create2 <name>\n");
            terminal_writestring("Available test processes: test1, test2\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return;
        }
        
        // External function declarations
        extern void test_process_1(void);
        extern void test_process_2(void);
        
        void (*entry_point)(void) = NULL;
        const char* proc_name = argv[2];
        
        // Select test process based on name
        if (simple_strcmp(proc_name, "test1") == 0) {
            entry_point = test_process_1;
        } else if (simple_strcmp(proc_name, "test2") == 0) {
            entry_point = test_process_2;
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_printf("Unknown test process: %s\n", proc_name);
            terminal_writestring("Available: test1, test2\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            return;
        }
        
        // Phase 2: Create process in table without execution
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_printf("Creating process '%s' in table...\n", proc_name);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        
        int pid = process_create_simple(entry_point, proc_name);
        if (pid != INVALID_PID) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            terminal_printf("Process '%s' created successfully with PID %d\n", proc_name, pid);
            terminal_writestring("Process is in READY state (not executed yet)\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("Failed to create process\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        }
        
    } else if (simple_strcmp(argv[1], "execute") == 0) {
        // Phase 3: Execute a ready process by PID
        if (argc < 3) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("Usage: proc execute <pid>\n");
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
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_printf("Executing process PID %d...\n", pid);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        
        int result = process_execute_simple(pid);
        if (result == 0) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            terminal_writestring("Process execution completed successfully!\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("Process execution failed!\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        }
        
    } else if (simple_strcmp(argv[1], "runall") == 0) {
        // Phase 4: Execute all ready processes in sequence
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("Starting batch execution of all ready processes...\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        
        int count = process_run_all_ready();
        
        if (count > 0) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            terminal_printf("Successfully executed %d processes!\n", count);
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            terminal_writestring("No ready processes found to execute.\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        }
        
    } else if (simple_strcmp(argv[1], "yield") == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
        terminal_writestring("Yielding CPU to next process...\n");
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        
        if (ready_queue_head) {
            process_yield();
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            terminal_writestring("Returned from process yield\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
            terminal_writestring("No processes in ready queue to yield to\n");
            terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
        }
        
    } else {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_printf("Unknown process command: %s\n", argv[1]);
        terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    }
}