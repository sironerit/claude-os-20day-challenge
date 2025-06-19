// ClaudeOS System Call Implementation - Day 8
// System call handlers and dispatch table

#include "syscall.h"
#include "kernel.h"
#include "process.h"
#include "string.h"
#include "../fs/simplefs.h"

// VGA helper function
static inline uint8_t vga_entry_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

// System call dispatch table
syscall_fn_t syscall_table[MAX_SYSCALLS] = {
    sys_hello,     // SYS_HELLO (0)
    sys_write,     // SYS_WRITE (1)
    sys_getpid,    // SYS_GETPID (2)
    sys_yield,     // SYS_YIELD (3)
    NULL,          // Reserved (4)
    sys_open,      // SYS_OPEN (5)
    sys_read_file, // SYS_READ_FILE (6)
    sys_write_file,// SYS_WRITE_FILE (7)
    sys_close,     // SYS_CLOSE (8)
    sys_mkdir,     // SYS_MKDIR (9)
    sys_list,      // SYS_LIST (10)
    sys_delete,    // SYS_DELETE (11)
    sys_fs_stats   // SYS_FS_STATS (12)
};

// Main system call handler (called from assembly)
int syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    // Validate system call number
    if (syscall_num >= MAX_SYSCALLS) {
        terminal_printf("[SYSCALL] Invalid syscall number: %d\n", syscall_num);
        return SYSCALL_INVALID;
    }
    
    // Get function pointer from dispatch table
    syscall_fn_t syscall_fn = syscall_table[syscall_num];
    if (!syscall_fn) {
        terminal_printf("[SYSCALL] Null syscall function for number: %d\n", syscall_num);
        return SYSCALL_ERROR;
    }
    
    // Call the system call function
    return syscall_fn(arg1, arg2, arg3);
}

// System Call Implementations

// SYS_HELLO (0) - Test system call
int sys_hello(uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    (void)arg1; (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("[SYSCALL] Hello from kernel! System calls working! ✅\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    
    return SYSCALL_SUCCESS;
}

// SYS_WRITE (1) - Write string to terminal
int sys_write(uint32_t str_ptr, uint32_t arg2, uint32_t arg3) {
    (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    // Basic pointer validation
    if (str_ptr == 0) {
        terminal_writestring("[SYSCALL] Error: NULL string pointer\n");
        return SYSCALL_ERROR;
    }
    
    // For now, assume the pointer is valid (in a real OS, we'd validate the address)
    const char* str = (const char*)str_ptr;
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("[PROCESS] ");
    terminal_writestring(str);
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    
    return strlen(str); // Return number of characters written
}

// SYS_GETPID (2) - Get current process ID
int sys_getpid(uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    (void)arg1; (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    if (current_process) {
        return current_process->pid;
    }
    
    return 0; // Kernel process ID
}

// SYS_YIELD (3) - Yield CPU to other processes
int sys_yield(uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    (void)arg1; (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    terminal_printf("[SYSCALL] Process %d yielding CPU\n", 
                   current_process ? current_process->pid : 0);
    
    // Call the scheduler to switch to another process
    yield();
    
    return SYSCALL_SUCCESS;
}

// C Wrapper Functions for easy calling

// Inline assembly wrapper for system calls
static inline int do_syscall(int syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    int result;
    
    asm volatile (
        "int $0x80"
        : "=a" (result)
        : "a" (syscall_num), "b" (arg1), "c" (arg2), "d" (arg3)
        : "memory"
    );
    
    return result;
}

// Wrapper functions
int syscall_hello(void) {
    return do_syscall(SYS_HELLO, 0, 0, 0);
}

int syscall_write(const char* str) {
    return do_syscall(SYS_WRITE, (uint32_t)str, 0, 0);
}

int syscall_getpid(void) {
    return do_syscall(SYS_GETPID, 0, 0, 0);
}

int syscall_yield(void) {
    return do_syscall(SYS_YIELD, 0, 0, 0);
}

// File System System Call Implementations (Day 9)

// SYS_OPEN (5) - Open file
int sys_open(uint32_t path_ptr, uint32_t mode, uint32_t arg3) {
    (void)arg3; // Suppress unused parameter warning
    
    if (path_ptr == 0) {
        return SYSCALL_ERROR;
    }
    
    const char* path = (const char*)path_ptr;
    uint8_t file_mode = (uint8_t)mode;
    
    int result = fs_open(path, file_mode);
    return result;
}

// SYS_READ_FILE (6) - Read from file
int sys_read_file(uint32_t fd, uint32_t buffer_ptr, uint32_t size) {
    if (buffer_ptr == 0) {
        return SYSCALL_ERROR;
    }
    
    void* buffer = (void*)buffer_ptr;
    int result = fs_read((int)fd, buffer, size);
    return result;
}

// SYS_WRITE_FILE (7) - Write to file
int sys_write_file(uint32_t fd, uint32_t buffer_ptr, uint32_t size) {
    if (buffer_ptr == 0) {
        return SYSCALL_ERROR;
    }
    
    const void* buffer = (const void*)buffer_ptr;
    int result = fs_write((int)fd, buffer, size);
    return result;
}

// SYS_CLOSE (8) - Close file
int sys_close(uint32_t fd, uint32_t arg2, uint32_t arg3) {
    (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    int result = fs_close((int)fd);
    return result;
}

// SYS_MKDIR (9) - Create directory
int sys_mkdir(uint32_t path_ptr, uint32_t arg2, uint32_t arg3) {
    (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    if (path_ptr == 0) {
        return SYSCALL_ERROR;
    }
    
    const char* path = (const char*)path_ptr;
    int result = fs_mkdir(path);
    return result;
}

// SYS_LIST (10) - List directory
int sys_list(uint32_t path_ptr, uint32_t arg2, uint32_t arg3) {
    (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    if (path_ptr == 0) {
        return SYSCALL_ERROR;
    }
    
    const char* path = (const char*)path_ptr;
    
    // For simplicity, just display the directory contents directly
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_printf("[FS] Directory listing for %s:\n", path);
    
    dir_entry_t entries[32];
    int count = fs_list(path, entries, 32);
    
    if (count < 0) {
        terminal_writestring("[FS] Error listing directory\n");
        return count;
    }
    
    for (int i = 0; i < count; i++) {
        if (entries[i].type == FS_TYPE_DIRECTORY) {
            terminal_printf("  [DIR]  %s\n", entries[i].name);
        } else {
            terminal_printf("  [FILE] %s (%d bytes)\n", entries[i].name, entries[i].size);
        }
    }
    
    terminal_printf("[FS] Total: %d entries\n", count);
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    
    return count;
}

// SYS_DELETE (11) - Delete file
int sys_delete(uint32_t path_ptr, uint32_t arg2, uint32_t arg3) {
    (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    if (path_ptr == 0) {
        return SYSCALL_ERROR;
    }
    
    const char* path = (const char*)path_ptr;
    // For now, just return success (delete not implemented)
    terminal_printf("[FS] Delete request for %s (not implemented)\n", path);
    return SYSCALL_SUCCESS;
}

// SYS_FS_STATS (12) - File system statistics
int sys_fs_stats(uint32_t arg1, uint32_t arg2, uint32_t arg3) {
    (void)arg1; (void)arg2; (void)arg3; // Suppress unused parameter warnings
    
    fs_dump_stats();
    return SYSCALL_SUCCESS;
}

// C wrapper functions for file system operations

int syscall_open(const char* path, uint8_t mode) {
    return do_syscall(SYS_OPEN, (uint32_t)path, (uint32_t)mode, 0);
}

int syscall_read_file(int fd, void* buffer, uint32_t size) {
    return do_syscall(SYS_READ_FILE, (uint32_t)fd, (uint32_t)buffer, size);
}

int syscall_write_file(int fd, const void* buffer, uint32_t size) {
    return do_syscall(SYS_WRITE_FILE, (uint32_t)fd, (uint32_t)buffer, size);
}

int syscall_close(int fd) {
    return do_syscall(SYS_CLOSE, (uint32_t)fd, 0, 0);
}

int syscall_mkdir(const char* path) {
    return do_syscall(SYS_MKDIR, (uint32_t)path, 0, 0);
}

int syscall_list(const char* path) {
    return do_syscall(SYS_LIST, (uint32_t)path, 0, 0);
}

int syscall_delete(const char* path) {
    return do_syscall(SYS_DELETE, (uint32_t)path, 0, 0);
}

int syscall_fs_stats(void) {
    return do_syscall(SYS_FS_STATS, 0, 0, 0);
}

// Initialize system call subsystem
void syscall_init(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("[SYSCALL] System call subsystem initialized\n");
    terminal_printf("[SYSCALL] %d system calls available:\n", MAX_SYSCALLS);
    terminal_writestring("[SYSCALL]   0: sys_hello - Test system call\n");
    terminal_writestring("[SYSCALL]   1: sys_write - Write string to terminal\n");
    terminal_writestring("[SYSCALL]   2: sys_getpid - Get process ID\n");
    terminal_writestring("[SYSCALL]   3: sys_yield - Yield CPU\n");
    terminal_writestring("[SYSCALL]   5: sys_open - Open file\n");
    terminal_writestring("[SYSCALL]   6: sys_read_file - Read from file\n");
    terminal_writestring("[SYSCALL]   7: sys_write_file - Write to file\n");
    terminal_writestring("[SYSCALL]   8: sys_close - Close file\n");
    terminal_writestring("[SYSCALL]   9: sys_mkdir - Create directory\n");
    terminal_writestring("[SYSCALL]  10: sys_list - List directory\n");
    terminal_writestring("[SYSCALL]  11: sys_delete - Delete file\n");
    terminal_writestring("[SYSCALL]  12: sys_fs_stats - File system stats\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}