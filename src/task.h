#ifndef TASK_H
#define TASK_H

#include <stdint.h>

// Structure defining saved register state
// MUST match the layout pushed by interrupt handlers
typedef struct registers {
    uint32_t ds;                                        // Data segment
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;  // Pushed by pusha
    uint32_t int_no, err_code;                         // Interrupt number and error code
    uint32_t eip, cs, eflags, useresp, ss;            // Pushed by CPU
} registers_t;

// Structure representing a task
typedef struct task {
    registers_t regs;       // Register state of the task
    uint32_t id;            // Unique task ID (PID)
    struct task *next;      // Pointer to next task in linked list
} task_t;

// Initialize the tasking system
void init_tasking();

// Create a new task
task_t* create_task(void (*entry_point)());

// Schedule next task and switch context
void schedule_and_switch(registers_t* regs);

// Get current task ID
uint32_t get_current_pid();

#endif // TASK_H