#include "task.h"
#include "mm.h"
#include "heap.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Current running task
volatile task_t *current_task = NULL;

// Ready queue (circular linked list)
volatile task_t *ready_queue = NULL;

// Next PID to assign
uint32_t next_pid = 0;

// Initialize the tasking system
void init_tasking() {
    __asm__ volatile("cli");

    // Create initial kernel task
    task_t *kernel_task = (task_t*)kmalloc(sizeof(task_t));
    if (!kernel_task) {
        return; // Failed to allocate
    }
    
    memset(kernel_task, 0, sizeof(task_t));
    kernel_task->id = next_pid++;
    kernel_task->next = kernel_task; // Point to itself (circular)
    
    current_task = kernel_task;
    ready_queue = kernel_task;

    __asm__ volatile("sti");
}

// Create a new task
task_t* create_task(void (*entry_point)()) {
    // Allocate task structure
    task_t *new_task = (task_t*)kmalloc(sizeof(task_t));
    if (!new_task) {
        return NULL;
    }
    
    memset(new_task, 0, sizeof(task_t));
    new_task->id = next_pid++;
    
    // Allocate stack (1 page = 4KB)
    uint32_t stack_bottom = pmm_alloc_frame();
    if (!stack_bottom) {
        kfree(new_task);
        return NULL;
    }
    
    uint32_t stack_top = stack_bottom + PAGE_SIZE;
    
    // Initialize registers for the new task
    new_task->regs.eip = (uint32_t)entry_point;
    new_task->regs.cs = 0x08;  // Kernel code segment
    new_task->regs.eflags = 0x202; // IF=1 (interrupts enabled)
    new_task->regs.esp = stack_top - 16; // Leave some room on stack
    new_task->regs.ss = 0x10;  // Kernel data segment
    new_task->regs.ds = 0x10;
    new_task->regs.es = 0x10;
    new_task->regs.fs = 0x10;
    new_task->regs.gs = 0x10;
    
    // Add to ready queue
    __asm__ volatile("cli");
    
    if (!ready_queue) {
        ready_queue = new_task;
        new_task->next = new_task;
    } else {
        // Find last task in circular list
        task_t *tmp = (task_t*)ready_queue;
        while(tmp->next != ready_queue) {
            tmp = tmp->next;
        }
        tmp->next = new_task;
        new_task->next = (task_t*)ready_queue;
    }
    
    __asm__ volatile("sti");
    
    return new_task;
}

// Schedule next task and perform context switch
void schedule_and_switch(registers_t* regs) {
    if (!current_task || !ready_queue) {
        return;
    }
    
    // Save current task state
    memcpy((void*)&current_task->regs, regs, sizeof(registers_t));
    
    // Select next task (round-robin)
    task_t *next_task = current_task->next;
    if (!next_task || next_task == current_task) {
        // Only one task, no switch needed
        return;
    }
    
    current_task = next_task;
    
    // Load next task state
    memcpy(regs, (void*)&current_task->regs, sizeof(registers_t));
}

// Get current task ID
uint32_t get_current_pid() {
    if (current_task) {
        return current_task->id;
    }
    return 0;
}