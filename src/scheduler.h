#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "task.h"

// Scheduling algorithms
#define SCHED_ROUND_ROBIN   0
#define SCHED_PRIORITY      1
#define SCHED_MULTILEVEL    2

// Task priorities (0 = highest, 31 = lowest)
#define PRIORITY_REALTIME   0
#define PRIORITY_HIGH       8
#define PRIORITY_NORMAL     16
#define PRIORITY_LOW        24
#define PRIORITY_IDLE       31

// Time slice in milliseconds
#define TIME_SLICE_MS       10  // 10ms per task

// Scheduler statistics
typedef struct {
    uint32_t context_switches;
    uint32_t total_tasks;
    uint32_t running_tasks;
    uint32_t blocked_tasks;
    uint32_t zombie_tasks;
    uint64_t total_ticks;
} scheduler_stats_t;

// Initialize the scheduler
void init_scheduler(uint32_t algorithm);

// Add task to scheduler
void scheduler_add_task(task_t* task, uint32_t priority);

// Remove task from scheduler
void scheduler_remove_task(task_t* task);

// Get next task to run
task_t* scheduler_get_next_task();

// Update task priority
void scheduler_set_priority(task_t* task, uint32_t priority);

// Block current task (remove from ready queue)
void scheduler_block_task(task_t* task);

// Unblock task (add back to ready queue)
void scheduler_unblock_task(task_t* task);

// Get scheduler statistics
void scheduler_get_stats(scheduler_stats_t* stats);

// Scheduler tick (called by timer interrupt)
void scheduler_tick();

// Print scheduler info
void scheduler_print_info();

#endif // SCHEDULER_H