#include "syscall.h"
#include "task.h"
#include "serial.h"
#include <stddef.h>

// External print function from kernel.c
extern void print(char* str);
extern void k_print_char(char c, uint8_t color);

// Syscall handler table
static syscall_handler_t syscall_table[SYSCALL_MAX + 1];

// Initialize the syscall system
void init_syscalls() {
    // Clear the syscall table
    for (int i = 0; i <= SYSCALL_MAX; i++) {
        syscall_table[i] = NULL;
    }
    
    // Register syscall handlers
    register_syscall(SYSCALL_EXIT, sys_exit);
    register_syscall(SYSCALL_WRITE, sys_write);
    register_syscall(SYSCALL_READ, sys_read);
    register_syscall(SYSCALL_FORK, sys_fork);
    register_syscall(SYSCALL_GETPID, sys_getpid);
    register_syscall(SYSCALL_YIELD, sys_yield);
    register_syscall(SYSCALL_SLEEP, sys_sleep);
}

// Register a syscall handler
void register_syscall(uint32_t num, syscall_handler_t handler) {
    if (num <= SYSCALL_MAX) {
        syscall_table[num] = handler;
    }
}

// The main syscall dispatcher (called from assembly)
uint32_t syscall_dispatcher(uint32_t syscall_num, uint32_t arg1, uint32_t arg2,
                            uint32_t arg3, uint32_t arg4, uint32_t arg5) {
    // Check if syscall number is valid
    if (syscall_num > SYSCALL_MAX) {
        return (uint32_t)-1; // Invalid syscall
    }
    
    // Check if handler is registered
    if (syscall_table[syscall_num] == NULL) {
        return (uint32_t)-1; // Unimplemented syscall
    }
    
    // Call the handler
    return syscall_table[syscall_num](arg1, arg2, arg3, arg4, arg5);
}

// Individual syscall implementations

// sys_exit: Terminate the current task
uint32_t sys_exit(uint32_t code, uint32_t unused1, uint32_t unused2,
                  uint32_t unused3, uint32_t unused4) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    (void)unused4;
    
    serial_print("[SYSCALL] exit(");
    // Print exit code (simplified)
    serial_print(")\n");
    
    task_exit();
    return code; // Should never reach here
}

// sys_write: Write to a file descriptor
uint32_t sys_write(uint32_t fd, uint32_t buf, uint32_t count,
                   uint32_t unused1, uint32_t unused2) {
    (void)unused1;
    (void)unused2;
    
    // For now, only support stdout (fd=1) and stderr (fd=2)
    if (fd != 1 && fd != 2) {
        return (uint32_t)-1; // Invalid fd
    }
    
    // Write to VGA buffer
    char* buffer = (char*)buf;
    for (uint32_t i = 0; i < count; i++) {
        if (buffer[i] == '\0') break;
        k_print_char(buffer[i], 0x0F);
    }
    
    return count;
}

// sys_read: Read from a file descriptor
uint32_t sys_read(uint32_t fd, uint32_t buf, uint32_t count,
                  uint32_t unused1, uint32_t unused2) {
    (void)fd;
    (void)buf;
    (void)count;
    (void)unused1;
    (void)unused2;
    
    // Not implemented yet
    return (uint32_t)-1;
}

// sys_fork: Create a new process
uint32_t sys_fork(uint32_t unused1, uint32_t unused2, uint32_t unused3,
                  uint32_t unused4, uint32_t unused5) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    (void)unused4;
    (void)unused5;
    
    // Not implemented yet
    return (uint32_t)-1;
}

// sys_getpid: Get current process ID
uint32_t sys_getpid(uint32_t unused1, uint32_t unused2, uint32_t unused3,
                    uint32_t unused4, uint32_t unused5) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    (void)unused4;
    (void)unused5;
    
    return get_current_pid();
}

// sys_yield: Yield the CPU to another task
uint32_t sys_yield(uint32_t unused1, uint32_t unused2, uint32_t unused3,
                   uint32_t unused4, uint32_t unused5) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    (void)unused4;
    (void)unused5;
    
    task_yield();
    return 0;
}

// sys_sleep: Sleep for specified milliseconds
uint32_t sys_sleep(uint32_t ms, uint32_t unused1, uint32_t unused2,
                   uint32_t unused3, uint32_t unused4) {
    (void)unused1;
    (void)unused2;
    (void)unused3;
    (void)unused4;
    
    task_sleep(ms);
    return 0;
}
