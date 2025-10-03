#include "process.h"
#include "task.h"
#include "mm.h"
#include "heap.h" // For kmalloc
#include <string.h> // For strncpy

typedef struct process {
    uint32_t pid;
    page_directory_t* page_dir;
    task_t* main_thread;
    struct process* next;
    char name[64];
    uint32_t state; // RUNNING, READY, BLOCKED, ZOMBIE
} process_t;

static process_t* process_list = NULL;
static process_t* current_process = NULL;

process_t* create_process(const char* name, void (*entry)()) {
    process_t* proc = (process_t*)kmalloc(sizeof(process_t));
    
    // Crea un nuovo address space
    // clone_page_directory is not defined yet, assuming it will be implemented in mm.c
    proc->page_dir = clone_page_directory(kernel_directory); 
    
    // Crea il thread principale
    proc->main_thread = create_task(entry);
    proc->main_thread->process = proc; // Assuming task_t has a process pointer
    
    strncpy(proc->name, name, 63);
    proc->name[63] = '\0'; // Ensure null termination
    proc->state = PROCESS_READY;
    
    // Aggiungi alla lista
    proc->next = process_list;
    process_list = proc;
    
    return proc;
}
