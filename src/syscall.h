#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

// System call numbers
#define SYSCALL_EXIT        0
#define SYSCALL_WRITE       1
#define SYSCALL_READ        2
#define SYSCALL_OPEN        3
#define SYSCALL_CLOSE       4
#define SYSCALL_FORK        5
#define SYSCALL_GETPID      6
#define SYSCALL_SLEEP       7
#define SYSCALL_YIELD       8
#define SYSCALL_KILL        9
#define SYSCALL_EXEC        10
#define SYSCALL_BRK         11  // Memory allocation
#define SYSCALL_MMAP        12
#define SYSCALL_MUNMAP      13
#define SYSCALL_GETTIME     14
#define SYSCALL_GETCWD      15
#define SYSCALL_CHDIR       16

// Maximum syscall number
#define SYSCALL_MAX         16

// Syscall handler function pointer type
typedef uint32_t (*syscall_handler_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

// Initialize the syscall system
void init_syscalls();

// Register a syscall handler
void register_syscall(uint32_t num, syscall_handler_t handler);

// The main syscall dispatcher (called from assembly)
uint32_t syscall_dispatcher(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, 
                            uint32_t arg3, uint32_t arg4, uint32_t arg5);

// Individual syscall implementations
uint32_t sys_exit(uint32_t code, uint32_t unused1, uint32_t unused2, 
                  uint32_t unused3, uint32_t unused4);
uint32_t sys_write(uint32_t fd, uint32_t buf, uint32_t count, 
                   uint32_t unused1, uint32_t unused2);
uint32_t sys_read(uint32_t fd, uint32_t buf, uint32_t count, 
                  uint32_t unused1, uint32_t unused2);
uint32_t sys_fork(uint32_t unused1, uint32_t unused2, uint32_t unused3, 
                  uint32_t unused4, uint32_t unused5);
uint32_t sys_getpid(uint32_t unused1, uint32_t unused2, uint32_t unused3, 
                    uint32_t unused4, uint32_t unused5);
uint32_t sys_yield(uint32_t unused1, uint32_t unused2, uint32_t unused3, 
                   uint32_t unused4, uint32_t unused5);
uint32_t sys_sleep(uint32_t ms, uint32_t unused1, uint32_t unused2, 
                   uint32_t unused3, uint32_t unused4);

#endif // SYSCALL_H