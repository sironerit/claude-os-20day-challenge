// ClaudeOS IPC (Inter-Process Communication) - Day 17 Implementation
#include "ipc.h"
#include "kernel.h"
#include "process.h"
#include "timer.h"
#include "heap.h"
#include "string.h"

// Global IPC data structures
message_t message_pool[MAX_MESSAGES];
semaphore_t semaphore_pool[MAX_SEMAPHORES];
shared_memory_t shared_memory_pool[8];  // Basic shared memory pool
int next_semaphore_id = 1;

// IPC initialization
void ipc_init(void) {
    // Initialize message pool
    for (int i = 0; i < MAX_MESSAGES; i++) {
        message_pool[i].sender_pid = INVALID_PID;
        message_pool[i].receiver_pid = INVALID_PID;
        message_pool[i].message_size = 0;
        message_pool[i].is_used = false;
        message_pool[i].timestamp = 0;
        // Clear message data
        for (int j = 0; j < MAX_MESSAGE_SIZE; j++) {
            message_pool[i].data[j] = 0;
        }
    }
    
    // Initialize semaphore pool
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        semaphore_pool[i].id = INVALID_SEMAPHORE_ID;
        semaphore_pool[i].value = 0;
        semaphore_pool[i].is_used = false;
        semaphore_pool[i].waiting_queue_head = NULL;
        semaphore_pool[i].waiting_queue_tail = NULL;
        semaphore_pool[i].creation_time = 0;
        // Clear semaphore name
        for (int j = 0; j < 32; j++) {
            semaphore_pool[i].name[j] = 0;
        }
    }
    
    // Initialize shared memory pool
    for (int i = 0; i < 8; i++) {
        shared_memory_pool[i].id = -1;
        shared_memory_pool[i].address = NULL;
        shared_memory_pool[i].size = 0;
        shared_memory_pool[i].owner_pid = INVALID_PID;
        shared_memory_pool[i].is_used = false;
    }
    
    next_semaphore_id = 1;
    
    terminal_printf("✅ IPC system initialized\n");
    terminal_printf("   - Message slots: %d\n", MAX_MESSAGES);
    terminal_printf("   - Semaphore slots: %d\n", MAX_SEMAPHORES);
    terminal_printf("   - Shared memory slots: 8\n");
}

// Message passing implementation
int ipc_send_message(int receiver_pid, const char* data, size_t size) {
    if (!data || size == 0 || size > MAX_MESSAGE_SIZE) {
        terminal_printf("❌ Invalid message data or size\n");
        return -1;
    }
    
    // Verify receiver process exists
    process_t* receiver = process_find(receiver_pid);
    if (!receiver) {
        terminal_printf("❌ Receiver process PID %d not found\n", receiver_pid);
        return -1;
    }
    
    // Find free message slot
    for (int i = 0; i < MAX_MESSAGES; i++) {
        if (!message_pool[i].is_used) {
            message_pool[i].sender_pid = current_process ? current_process->pid : 0;
            message_pool[i].receiver_pid = receiver_pid;
            message_pool[i].message_size = size;
            message_pool[i].is_used = true;
            message_pool[i].timestamp = get_uptime_seconds();
            
            // Copy message data
            for (size_t j = 0; j < size && j < MAX_MESSAGE_SIZE; j++) {
                message_pool[i].data[j] = data[j];
            }
            
            terminal_printf("✅ Message sent to PID %d (slot %d, %d bytes)\n", 
                           receiver_pid, i, (int)size);
            return i;  // Return message slot ID
        }
    }
    
    terminal_printf("❌ No free message slots available\n");
    return -1;
}

int ipc_receive_message(int sender_pid, char* buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        terminal_printf("❌ Invalid receive buffer\n");
        return -1;
    }
    
    int receiver_pid = current_process ? current_process->pid : 0;
    
    // Search for message
    for (int i = 0; i < MAX_MESSAGES; i++) {
        if (message_pool[i].is_used && 
            message_pool[i].receiver_pid == receiver_pid &&
            (sender_pid == -1 || message_pool[i].sender_pid == sender_pid)) {
            
            // Copy message data
            size_t copy_size = message_pool[i].message_size;
            if (copy_size > buffer_size - 1) {
                copy_size = buffer_size - 1;
            }
            
            for (size_t j = 0; j < copy_size; j++) {
                buffer[j] = message_pool[i].data[j];
            }
            buffer[copy_size] = '\0';  // Null terminate
            
            int sender = message_pool[i].sender_pid;
            
            // Mark message as free
            message_pool[i].is_used = false;
            message_pool[i].sender_pid = INVALID_PID;
            message_pool[i].receiver_pid = INVALID_PID;
            
            terminal_printf("✅ Message received from PID %d (%d bytes)\n", 
                           sender, (int)copy_size);
            return sender;  // Return sender PID
        }
    }
    
    terminal_printf("❌ No messages found from PID %d\n", sender_pid);
    return -1;
}

int ipc_message_count(int pid) {
    int count = 0;
    for (int i = 0; i < MAX_MESSAGES; i++) {
        if (message_pool[i].is_used && message_pool[i].receiver_pid == pid) {
            count++;
        }
    }
    return count;
}

void ipc_list_messages(void) {
    terminal_writestring("📬 Message Queue Status:\n");
    terminal_writestring("Slot Sender Receiver Size  Data\n");
    terminal_writestring("---- ------ -------- ----  ----\n");
    
    bool found_any = false;
    for (int i = 0; i < MAX_MESSAGES; i++) {
        if (message_pool[i].is_used) {
            found_any = true;
            // Simple number display without printf formatting
            char slot_str[8], sender_str[8], receiver_str[8], size_str[8];
            itoa(i, slot_str, 10);
            itoa(message_pool[i].sender_pid, sender_str, 10);
            itoa(message_pool[i].receiver_pid, receiver_str, 10);
            itoa((int)message_pool[i].message_size, size_str, 10);
            
            terminal_writestring(slot_str);
            terminal_writestring("   ");
            terminal_writestring(sender_str);
            terminal_writestring("    ");
            terminal_writestring(receiver_str);
            terminal_writestring("      ");
            terminal_writestring(size_str);
            terminal_writestring("   \"");
            
            // Print first 20 chars of message
            for (int j = 0; j < 20 && j < (int)message_pool[i].message_size; j++) {
                if (message_pool[i].data[j] >= 32 && message_pool[i].data[j] <= 126) {
                    terminal_putchar(message_pool[i].data[j]);
                } else {
                    terminal_putchar('.');
                }
            }
            terminal_writestring("\"\n");
        }
    }
    
    if (!found_any) {
        terminal_writestring("No messages in queue\n");
    }
}

// Semaphore implementation
int ipc_create_semaphore(const char* name, int initial_value) {
    if (!name || initial_value < 0) {
        terminal_printf("❌ Invalid semaphore parameters\n");
        return INVALID_SEMAPHORE_ID;
    }
    
    // Find free semaphore slot
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (!semaphore_pool[i].is_used) {
            semaphore_pool[i].id = next_semaphore_id++;
            semaphore_pool[i].value = initial_value;
            semaphore_pool[i].is_used = true;
            semaphore_pool[i].waiting_queue_head = NULL;
            semaphore_pool[i].waiting_queue_tail = NULL;
            semaphore_pool[i].creation_time = get_uptime_seconds();
            
            // Copy name
            int j;
            for (j = 0; j < 31 && name[j] != '\0'; j++) {
                semaphore_pool[i].name[j] = name[j];
            }
            semaphore_pool[i].name[j] = '\0';
            
            terminal_printf("✅ Semaphore '%s' created (ID: %d, value: %d)\n", 
                           name, semaphore_pool[i].id, initial_value);
            return semaphore_pool[i].id;
        }
    }
    
    terminal_printf("❌ No free semaphore slots available\n");
    return INVALID_SEMAPHORE_ID;
}

semaphore_t* ipc_find_semaphore(int semaphore_id) {
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (semaphore_pool[i].is_used && semaphore_pool[i].id == semaphore_id) {
            return &semaphore_pool[i];
        }
    }
    return NULL;
}

int ipc_semaphore_wait(int semaphore_id) {
    semaphore_t* sem = ipc_find_semaphore(semaphore_id);
    if (!sem) {
        terminal_printf("❌ Semaphore ID %d not found\n", semaphore_id);
        return -1;
    }
    
    if (sem->value > 0) {
        sem->value--;
        terminal_printf("✅ Semaphore %d acquired (value: %d)\n", 
                       semaphore_id, sem->value);
        return 0;
    } else {
        // Add current process to waiting queue
        if (current_process) {
            current_process->state = PROCESS_BLOCKED;
            ipc_add_to_waiting_queue(sem, current_process);
            terminal_writestring("⏳ Process ");
            char pid_str[8];
            itoa(current_process->pid, pid_str, 10);
            terminal_writestring(pid_str);
            terminal_writestring(" waiting on semaphore ");
            char sem_str[8];
            itoa(semaphore_id, sem_str, 10);
            terminal_writestring(sem_str);
            terminal_writestring("\n");
            // In real implementation, this would trigger process switch
            return 1;  // Indicate blocking
        } else {
            terminal_writestring("⏳ Kernel process waiting on semaphore ");
            char sem_str[8];
            itoa(semaphore_id, sem_str, 10);
            terminal_writestring(sem_str);
            terminal_writestring(" (would block in real system)\n");
            return 1;
        }
    }
}

int ipc_semaphore_signal(int semaphore_id) {
    semaphore_t* sem = ipc_find_semaphore(semaphore_id);
    if (!sem) {
        terminal_printf("❌ Semaphore ID %d not found\n", semaphore_id);
        return -1;
    }
    
    // Check if any process is waiting
    process_t* waiting_process = ipc_remove_from_waiting_queue(sem);
    if (waiting_process) {
        waiting_process->state = PROCESS_READY;
        terminal_printf("✅ Process %d unblocked from semaphore %d\n", 
                       waiting_process->pid, semaphore_id);
    } else {
        sem->value++;
        terminal_printf("✅ Semaphore %d signaled (value: %d)\n", 
                       semaphore_id, sem->value);
    }
    
    return 0;
}

int ipc_destroy_semaphore(int semaphore_id) {
    semaphore_t* sem = ipc_find_semaphore(semaphore_id);
    if (!sem) {
        terminal_printf("❌ Semaphore ID %d not found\n", semaphore_id);
        return -1;
    }
    
    // Wake up all waiting processes
    while (sem->waiting_queue_head) {
        process_t* waiting_process = ipc_remove_from_waiting_queue(sem);
        if (waiting_process) {
            waiting_process->state = PROCESS_READY;
            terminal_printf("⚠️  Process %d unblocked (semaphore destroyed)\n", 
                           waiting_process->pid);
        }
    }
    
    // Clear semaphore
    sem->is_used = false;
    sem->id = INVALID_SEMAPHORE_ID;
    sem->value = 0;
    
    terminal_printf("✅ Semaphore %d destroyed\n", semaphore_id);
    return 0;
}

void ipc_list_semaphores(void) {
    terminal_writestring("🔒 Semaphore Status:\n");
    terminal_writestring("ID   Name                Value Waiting\n");
    terminal_writestring("---- -------------------- ----- -------\n");
    
    bool found_any = false;
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (semaphore_pool[i].is_used) {
            found_any = true;
            
            // Count waiting processes
            int waiting_count = 0;
            process_t* p = semaphore_pool[i].waiting_queue_head;
            while (p) {
                waiting_count++;
                p = p->next;
            }
            
            // Simple display without printf formatting
            char id_str[8], value_str[8], waiting_str[8];
            itoa(semaphore_pool[i].id, id_str, 10);
            itoa(semaphore_pool[i].value, value_str, 10);
            itoa(waiting_count, waiting_str, 10);
            
            terminal_writestring(id_str);
            terminal_writestring("  ");
            terminal_writestring(semaphore_pool[i].name);
            
            // Pad name to 20 chars
            int name_len = strlen(semaphore_pool[i].name);
            for (int j = name_len; j < 20; j++) {
                terminal_writestring(" ");
            }
            
            terminal_writestring(" ");
            terminal_writestring(value_str);
            terminal_writestring("   ");
            terminal_writestring(waiting_str);
            terminal_writestring("\n");
        }
    }
    
    if (!found_any) {
        terminal_writestring("No semaphores created\n");
    }
}

// Queue management for semaphores
void ipc_add_to_waiting_queue(semaphore_t* sem, process_t* process) {
    if (!sem || !process) return;
    
    process->next = NULL;
    
    if (!sem->waiting_queue_head) {
        sem->waiting_queue_head = process;
        sem->waiting_queue_tail = process;
    } else {
        sem->waiting_queue_tail->next = process;
        sem->waiting_queue_tail = process;
    }
}

process_t* ipc_remove_from_waiting_queue(semaphore_t* sem) {
    if (!sem || !sem->waiting_queue_head) {
        return NULL;
    }
    
    process_t* process = sem->waiting_queue_head;
    sem->waiting_queue_head = process->next;
    
    if (!sem->waiting_queue_head) {
        sem->waiting_queue_tail = NULL;
    }
    
    process->next = NULL;
    return process;
}

// IPC statistics
void ipc_stats(void) {
    terminal_writestring("📊 IPC System Statistics:\n");
    
    int used_messages = 0;
    int used_semaphores = 0;
    
    for (int i = 0; i < MAX_MESSAGES; i++) {
        if (message_pool[i].is_used) used_messages++;
    }
    
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (semaphore_pool[i].is_used) used_semaphores++;
    }
    
    terminal_printf("Messages: %d/%d used\n", used_messages, MAX_MESSAGES);
    terminal_printf("Semaphores: %d/%d used\n", used_semaphores, MAX_SEMAPHORES);
    terminal_printf("Next semaphore ID: %d\n", next_semaphore_id);
}

// IPC command handler
void ipc_command_handler(int argc, char argv[][64]) {
    if (argc < 2) {
        terminal_writestring("IPC Commands:\n");
        terminal_writestring("  ipc init        - Initialize IPC system\n");
        terminal_writestring("  ipc send <pid> <message>  - Send message\n");
        terminal_writestring("  ipc recv [pid]  - Receive message\n");
        terminal_writestring("  ipc messages    - List all messages\n");
        terminal_writestring("  ipc sem create <name> <value> - Create semaphore\n");
        terminal_writestring("  ipc sem wait <id>     - Wait on semaphore\n");
        terminal_writestring("  ipc sem signal <id>   - Signal semaphore\n");
        terminal_writestring("  ipc sem list    - List semaphores\n");
        terminal_writestring("  ipc sem destroy <id>  - Destroy semaphore\n");
        terminal_writestring("  ipc stats       - Show IPC statistics\n");
        return;
    }
    
    if (strcmp(argv[1], "init") == 0) {
        ipc_init();
    }
    else if (strcmp(argv[1], "send") == 0) {
        if (argc < 4) {
            terminal_writestring("Usage: ipc send <pid> <message>\n");
            return;
        }
        int pid = atoi(argv[2]);
        ipc_send_message(pid, argv[3], strlen(argv[3]));
    }
    else if (strcmp(argv[1], "recv") == 0) {
        char buffer[MAX_MESSAGE_SIZE];
        int sender_pid = (argc >= 3) ? atoi(argv[2]) : -1;
        int result = ipc_receive_message(sender_pid, buffer, sizeof(buffer));
        if (result >= 0) {
            terminal_printf("Received: \"%s\"\n", buffer);
        }
    }
    else if (strcmp(argv[1], "messages") == 0) {
        ipc_list_messages();
    }
    else if (strcmp(argv[1], "sem") == 0) {
        if (argc < 3) {
            terminal_writestring("Usage: ipc sem <create|wait|signal|list|destroy>\n");
            return;
        }
        
        if (strcmp(argv[2], "create") == 0) {
            if (argc < 5) {
                terminal_writestring("Usage: ipc sem create <name> <value>\n");
                return;
            }
            int value = atoi(argv[4]);
            ipc_create_semaphore(argv[3], value);
        }
        else if (strcmp(argv[2], "wait") == 0) {
            if (argc < 4) {
                terminal_writestring("Usage: ipc sem wait <id>\n");
                return;
            }
            int id = atoi(argv[3]);
            ipc_semaphore_wait(id);
        }
        else if (strcmp(argv[2], "signal") == 0) {
            if (argc < 4) {
                terminal_writestring("Usage: ipc sem signal <id>\n");
                return;
            }
            int id = atoi(argv[3]);
            ipc_semaphore_signal(id);
        }
        else if (strcmp(argv[2], "list") == 0) {
            ipc_list_semaphores();
        }
        else if (strcmp(argv[2], "destroy") == 0) {
            if (argc < 4) {
                terminal_writestring("Usage: ipc sem destroy <id>\n");
                return;
            }
            int id = atoi(argv[3]);
            ipc_destroy_semaphore(id);
        }
    }
    else if (strcmp(argv[1], "stats") == 0) {
        ipc_stats();
    }
    else {
        terminal_printf("Unknown IPC command: %s\n", argv[1]);
    }
}