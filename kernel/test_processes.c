// ClaudeOS Test Processes for IPC - Day 17 Implementation
#include "kernel.h"
#include "process.h"
#include "ipc.h"
#include "timer.h"

// Test process IPC sender: Message sender
void test_process_ipc_sender(void) {
    terminal_printf("🔵 IPC Sender Process started (PID: %d)\n", 
                   current_process ? current_process->pid : 0);
    
    // Simulate work
    for (int i = 0; i < 3; i++) {
        terminal_printf("🔵 IPC Sender: Working... (%d/3)\n", i + 1);
        
        // Send message to process 2 (PID 2)
        char message[64];
        terminal_printf("🔵 IPC Sender: Sending message to Process 2\n");
        
        // Simple message construction
        message[0] = 'H'; message[1] = 'e'; message[2] = 'l'; message[3] = 'l';
        message[4] = 'o'; message[5] = ' '; message[6] = 'f'; message[7] = 'r';
        message[8] = 'o'; message[9] = 'm'; message[10] = ' '; message[11] = 'P';
        message[12] = '1'; message[13] = '\0';
        
        int result = ipc_send_message(2, message, 13);
        if (result >= 0) {
            terminal_printf("🔵 IPC Sender: Message sent successfully\n");
        } else {
            terminal_printf("🔵 IPC Sender: Failed to send message\n");
        }
        
        // Yield to other processes
        process_yield();
    }
    
    terminal_printf("🔵 IPC Sender: Work completed, exiting\n");
    process_exit(0);
}

// Test process IPC receiver: Message receiver
void test_process_ipc_receiver(void) {
    terminal_printf("🟢 IPC Receiver Process started (PID: %d)\n", 
                   current_process ? current_process->pid : 0);
    
    // Simulate work and message receiving
    for (int i = 0; i < 5; i++) {
        terminal_printf("🟢 IPC Receiver: Working... (%d/5)\n", i + 1);
        
        // Try to receive message
        char buffer[256];
        int sender = ipc_receive_message(-1, buffer, sizeof(buffer));
        if (sender >= 0) {
            terminal_printf("🟢 IPC Receiver: Received message from PID %d: \"%s\"\n", 
                           sender, buffer);
        } else {
            terminal_printf("🟢 IPC Receiver: No messages waiting\n");
        }
        
        // Yield to other processes
        process_yield();
    }
    
    terminal_printf("🟢 IPC Receiver: Work completed, exiting\n");
    process_exit(0);
}

// Test process 3: Semaphore test (producer)
void test_process_producer(void) {
    terminal_printf("🟡 Producer Process started (PID: %d)\n", 
                   current_process ? current_process->pid : 0);
    
    // Create a semaphore for resource sharing
    int sem_id = ipc_create_semaphore("test_resource", 1);
    if (sem_id < 0) {
        terminal_printf("🟡 Producer: Failed to create semaphore\n");
        process_exit(1);
        return;
    }
    
    for (int i = 0; i < 3; i++) {
        terminal_printf("🟡 Producer: Requesting resource (%d/3)\n", i + 1);
        
        // Wait for resource
        int wait_result = ipc_semaphore_wait(sem_id);
        if (wait_result == 0) {
            terminal_printf("🟡 Producer: Got resource, using it...\n");
            
            // Simulate resource usage
            process_yield(); // Let other processes run
            
            terminal_printf("🟡 Producer: Done with resource, releasing...\n");
            ipc_semaphore_signal(sem_id);
        } else {
            terminal_printf("🟡 Producer: Failed to acquire resource\n");
        }
        
        process_yield();
    }
    
    terminal_printf("🟡 Producer: Work completed, exiting\n");
    process_exit(0);
}

// Test process 4: Semaphore test (consumer)
void test_process_consumer(void) {
    terminal_printf("🟠 Consumer Process started (PID: %d)\n", 
                   current_process ? current_process->pid : 0);
    
    // Find the existing semaphore (created by producer)
    // Note: In a real implementation, we'd have semaphore discovery
    int sem_id = 1; // Assume first semaphore created
    
    for (int i = 0; i < 2; i++) {
        terminal_printf("🟠 Consumer: Requesting resource (%d/2)\n", i + 1);
        
        // Wait for resource
        int wait_result = ipc_semaphore_wait(sem_id);
        if (wait_result == 0) {
            terminal_printf("🟠 Consumer: Got resource, using it...\n");
            
            // Simulate resource usage
            process_yield(); // Let other processes run
            
            terminal_printf("🟠 Consumer: Done with resource, releasing...\n");
            ipc_semaphore_signal(sem_id);
        } else if (wait_result == 1) {
            terminal_printf("🟠 Consumer: Waiting for resource...\n");
            // In real implementation, this process would be blocked
        } else {
            terminal_printf("🟠 Consumer: Failed to access semaphore\n");
        }
        
        process_yield();
    }
    
    terminal_printf("🟠 Consumer: Work completed, exiting\n");
    process_exit(0);
}

// Simple test process for multitasking
void test_process_simple(void) {
    terminal_printf("⭐ Simple Test Process started (PID: %d)\n", 
                   current_process ? current_process->pid : 0);
    
    for (int i = 0; i < 5; i++) {
        terminal_printf("⭐ Simple Process: Counter = %d\n", i);
        process_yield(); // Cooperative multitasking
    }
    
    terminal_printf("⭐ Simple Process: Completed, exiting\n");
    process_exit(0);
}