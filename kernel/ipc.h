// ClaudeOS IPC (Inter-Process Communication) - Day 17 Implementation
// Advanced process communication and synchronization

#ifndef IPC_H
#define IPC_H

#include "types.h"
#include "process.h"

// IPC configuration constants
#define MAX_MESSAGES 16
#define MAX_MESSAGE_SIZE 256
#define MAX_SEMAPHORES 8
#define INVALID_SEMAPHORE_ID -1

// Message structure for IPC
typedef struct {
    int sender_pid;                    // Sender process ID
    int receiver_pid;                  // Receiver process ID
    size_t message_size;               // Message size in bytes
    char data[MAX_MESSAGE_SIZE];       // Message data
    bool is_used;                      // Message slot usage flag
    uint32_t timestamp;                // Message timestamp
} message_t;

// Semaphore structure for process synchronization
typedef struct {
    int id;                            // Semaphore ID
    int value;                         // Semaphore value (resource count)
    bool is_used;                      // Semaphore slot usage flag
    process_t* waiting_queue_head;     // Waiting processes queue head
    process_t* waiting_queue_tail;     // Waiting processes queue tail
    char name[32];                     // Semaphore name
    uint32_t creation_time;            // Creation timestamp
} semaphore_t;

// Shared memory structure
typedef struct {
    int id;                            // Shared memory ID
    void* address;                     // Memory address
    size_t size;                       // Memory size
    int owner_pid;                     // Owner process ID
    bool is_used;                      // Usage flag
    char name[32];                     // Shared memory name
} shared_memory_t;

// Global IPC data structures
extern message_t message_pool[MAX_MESSAGES];
extern semaphore_t semaphore_pool[MAX_SEMAPHORES];
extern int next_semaphore_id;

// IPC initialization
void ipc_init(void);

// Message passing functions
int ipc_send_message(int receiver_pid, const char* data, size_t size);
int ipc_receive_message(int sender_pid, char* buffer, size_t buffer_size);
int ipc_message_count(int pid);
void ipc_list_messages(void);

// Semaphore functions
int ipc_create_semaphore(const char* name, int initial_value);
int ipc_semaphore_wait(int semaphore_id);
int ipc_semaphore_signal(int semaphore_id);
int ipc_destroy_semaphore(int semaphore_id);
void ipc_list_semaphores(void);
semaphore_t* ipc_find_semaphore(int semaphore_id);

// Shared memory functions (basic implementation)
int ipc_create_shared_memory(const char* name, size_t size);
void* ipc_attach_shared_memory(int shared_mem_id);
int ipc_detach_shared_memory(int shared_mem_id);

// IPC command handlers
void ipc_command_handler(int argc, char argv[][64]);

// Helper functions
void ipc_add_to_waiting_queue(semaphore_t* sem, process_t* process);
process_t* ipc_remove_from_waiting_queue(semaphore_t* sem);
void ipc_stats(void);

#endif // IPC_H