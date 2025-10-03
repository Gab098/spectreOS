#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "mm.h" // For page_directory_t
#include "task.h" // For task_t

// Process states
#define PROCESS_RUNNING 0
#define PROCESS_READY   1
#define PROCESS_BLOCKED 2
#define PROCESS_ZOMBIE  3

typedef struct process {
    uint32_t pid;
    page_directory_t* page_dir;
    task_t* main_thread;
    struct process* next;
    char name[64];
    uint32_t state; // RUNNING, READY, BLOCKED, ZOMBIE
} process_t;

process_t* create_process(const char* name, void (*entry)());

#endif
