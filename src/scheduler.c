#include "scheduler.h"
#include "task.h"
#include "serial.h"
#include <stddef.h>

// External print function from kernel.c
extern void print(char* str);
extern void print_dec(uint32_t n);

// Scheduler state
static uint32_t scheduler_algorithm = SCHED_ROUND_ROBIN;
static scheduler_stats_t scheduler_stats;

// Priority queues (simplified - just track tasks)
static task_t* high_priority_queue = NULL;
static task_t* normal_priority_queue = NULL;
static task_t* low_priority_queue = NULL;

// Initialize the scheduler
void init_scheduler(uint32_t algorithm) {
    scheduler_algorithm = algorithm;
    
    // Initialize statistics
    scheduler_stats.total_ticks = 0;
    scheduler_stats.context_switches = 0;
    scheduler_stats.idle_ticks = 0;
    
    serial_print("[SCHEDULER] Initialized with algorithm: ");
    if (algorithm == SCHED_ROUND_ROBIN) {
        serial_print("Round Robin\n");
    } else if (algorithm == SCHED_PRIORITY) {
        serial_print("Priority-based\n");
    } else {
        serial_print("Unknown\n");
    }
}

// Add task to scheduler
void scheduler_add_task(task_t* task, uint32_t priority) {
    if (!task) return;
    
    // For now, just acknowledge the task
    // In a full implementation, we would add it to the appropriate priority queue
    (void)priority;
    
    serial_print("[SCHEDULER] Added task PID=");
    // Print task ID (simplified)
    serial_print(" with priority=");
    // Print priority (simplified)
    serial_print("\n");
}

// Remove task from scheduler
void scheduler_remove_task(task_t* task) {
    if (!task) return;
    
    // Stub implementation
    serial_print("[SCHEDULER] Removed task\n");
}

// Get next task to run
task_t* scheduler_get_next_task() {
    // Stub implementation - return NULL to let task.c use its own round-robin
    return NULL;
}

// Update task priority
void scheduler_set_priority(task_t* task, uint32_t priority) {
    if (!task) return;
    
    task->priority = priority;
    serial_print("[SCHEDULER] Updated task priority\n");
}

// Block current task (remove from ready queue)
void scheduler_block_task(task_t* task) {
    if (!task) return;
    
    task->state = TASK_BLOCKED;
    serial_print("[SCHEDULER] Blocked task\n");
}

// Unblock task (add back to ready queue)
void scheduler_unblock_task(task_t* task) {
    if (!task) return;
    
    task->state = TASK_READY;
    serial_print("[SCHEDULER] Unblocked task\n");
}

// Get scheduler statistics
void scheduler_get_stats(scheduler_stats_t* stats) {
    if (!stats) return;
    
    stats->total_ticks = scheduler_stats.total_ticks;
    stats->context_switches = scheduler_stats.context_switches;
    stats->idle_ticks = scheduler_stats.idle_ticks;
}

// Scheduler tick (called by timer interrupt)
void scheduler_tick() {
    scheduler_stats.total_ticks++;
}

// Print scheduler info
void scheduler_print_info() {
    print("\n=== Scheduler Info ===\n");
    print("Algorithm: ");
    
    if (scheduler_algorithm == SCHED_ROUND_ROBIN) {
        print("Round Robin\n");
    } else if (scheduler_algorithm == SCHED_PRIORITY) {
        print("Priority-based\n");
    } else {
        print("Unknown\n");
    }
    
    print("Total ticks: ");
    print_dec(scheduler_stats.total_ticks);
    print("\n");
    
    print("Context switches: ");
    print_dec(scheduler_stats.context_switches);
    print("\n");
    
    print("======================\n\n");
}
