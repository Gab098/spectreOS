#include "task.h"
#include "mm.h"
#include "heap.h"
#include "scheduler.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Puntatore al task attualmente in esecuzione
volatile task_t *current_task = NULL;

// Coda dei task pronti (lista concatenata circolare)
volatile task_t *ready_queue = NULL;

// Prossimo ID da assegnare
static uint32_t next_pid = 1;

// Lista di tutti i task
static task_t *all_tasks = NULL;

// Funzione per lo scheduling e il context switch
void schedule_and_switch(registers_t* regs) {
    if (!current_task || !ready_queue) {
        return; // Nessun task da schedulare
    }

    // Salva lo stato del task corrente nei suoi registri
    current_task->regs.edi = regs->edi;
    current_task->regs.esi = regs->esi;
    current_task->regs.ebp = regs->ebp;
    current_task->regs.esp = regs->esp;
    current_task->regs.ebx = regs->ebx;
    current_task->regs.edx = regs->edx;
    current_task->regs.ecx = regs->ecx;
    current_task->regs.eax = regs->eax;
    current_task->regs.eip = regs->eip;
    current_task->regs.cs = regs->cs;
    current_task->regs.eflags = regs->eflags;
    current_task->regs.useresp = regs->useresp;
    current_task->regs.ss = regs->ss;
    current_task->regs.ds = regs->ds;
    current_task->regs.es = regs->es;
    current_task->regs.fs = regs->fs;
    current_task->regs.gs = regs->gs;
    
    // Update CPU time
    ((task_t*)current_task)->cpu_time++;

    // Seleziona il prossimo task usando lo scheduler se disponibile
    // Altrimenti usa round-robin semplice
    task_t* next = NULL;
    
    // Try scheduler first (if initialized)
    extern scheduler_stats_t;
    // next = scheduler_get_next_task(); // Uncomment when scheduler is fully integrated
    
    // Fallback to simple round-robin
    if (!next) {
        if (current_task->next) {
            next = (task_t*)current_task->next;
        } else {
            next = (task_t*)ready_queue;
        }
    }
    
    if (next) {
        current_task = next;
    }

    // Carica lo stato del nuovo task
    regs->edi = current_task->regs.edi;
    regs->esi = current_task->regs.esi;
    regs->ebp = current_task->regs.ebp;
    regs->esp = current_task->regs.esp;
    regs->ebx = current_task->regs.ebx;
    regs->edx = current_task->regs.edx;
    regs->ecx = current_task->regs.ecx;
    regs->eax = current_task->regs.eax;
    regs->eip = current_task->regs.eip;
    regs->cs = current_task->regs.cs;
    regs->eflags = current_task->regs.eflags;
    regs->useresp = current_task->regs.useresp;
    regs->ss = current_task->regs.ss;
    regs->ds = current_task->regs.ds;
    regs->es = current_task->regs.es;
    regs->fs = current_task->regs.fs;
    regs->gs = current_task->regs.gs;
    
    // Notify scheduler (if available)
    // scheduler_tick();
}

void init_tasking() {
    __asm__ volatile("cli");

    // Crea il task iniziale per il kernel
    current_task = (task_t*)kmalloc(sizeof(task_t));
    if (!current_task) {
        return; // Out of memory
    }

    memset(current_task, 0, sizeof(task_t));
    
    current_task->id = next_pid++;
    current_task->state = TASK_RUNNING;
    current_task->priority = 16; // Normal priority
    current_task->ring = 0; // Kernel mode
    current_task->time_slice = 10;
    current_task->cpu_time = 0;
    strcpy(((task_t*)current_task)->name, "kernel_idle");
    
    // Inizializza i registri del kernel task
    memset(&current_task->regs, 0, sizeof(registers_t));
    current_task->regs.cs = 0x08;  // Kernel code segment
    current_task->regs.ds = 0x10;  // Kernel data segment
    current_task->regs.es = 0x10;
    current_task->regs.fs = 0x10;
    current_task->regs.gs = 0x10;
    current_task->regs.ss = 0x10;
    current_task->regs.eflags = 0x202; // IF=1 (interrupts enabled)
    
    // Lista circolare: punta a se stesso
    ((task_t*)current_task)->next = (task_t*)current_task;
    ((task_t*)current_task)->parent = NULL;
    ready_queue = current_task;
    
    // Add to all_tasks list
    all_tasks = (task_t*)current_task;

    __asm__ volatile("sti");
}

task_t* create_task(void (*entry_point)()) {
    return create_task_ex(entry_point, 16, 0, "unnamed");
}

task_t* create_task_ex(void (*entry_point)(), uint32_t priority, uint32_t ring, const char* name) {
    __asm__ volatile("cli");

    task_t *new_task = (task_t*)kmalloc(sizeof(task_t));
    if (!new_task) {
        __asm__ volatile("sti");
        return NULL;
    }

    memset(new_task, 0, sizeof(task_t));
    new_task->id = next_pid++;
    new_task->state = TASK_READY;
    new_task->priority = priority;
    new_task->ring = ring;
    new_task->time_slice = 10; // 10 ticks
    new_task->cpu_time = 0;
    
    // Copy name
    strncpy(new_task->name, name, 63);
    new_task->name[63] = '\0';
    
    // Alloca 2 pagine (8KB) per lo stack del nuovo task
    uint32_t stack_phys = pmm_alloc_frame();
    if (!stack_phys) {
        kfree(new_task);
        __asm__ volatile("sti");
        return NULL;
    }
    
    // Mappa lo stack nello spazio di indirizzamento del kernel per ora
    uint32_t stack_virt = stack_phys;
    uint32_t stack_top = stack_virt + PAGE_SIZE;
    
    // Alloca una seconda pagina per più spazio stack
    uint32_t stack_phys2 = pmm_alloc_frame();
    if (stack_phys2) {
        stack_top = stack_phys2 + PAGE_SIZE;
    }
    
    // Salva stack pointers
    if (ring == 0) {
        // Kernel mode
        new_task->kernel_stack = stack_top;
        new_task->user_stack = 0;
    } else {
        // User mode
        new_task->user_stack = stack_top;
        // Alloca anche kernel stack per syscalls
        uint32_t kstack_phys = pmm_alloc_frame();
        if (kstack_phys) {
            new_task->kernel_stack = kstack_phys + PAGE_SIZE;
        }
    }
    
    // Inizializza i registri per il nuovo task
    new_task->regs.eip = (uint32_t)entry_point;
    new_task->regs.eflags = 0x202; // IF=1 (interrupts enabled)
    new_task->regs.esp = stack_top - 16; // Lascia spazio per sicurezza
    new_task->regs.ebp = new_task->regs.esp;
    
    if (ring == 0) {
        // Kernel mode
        new_task->regs.cs = 0x08;   // Kernel code segment
        new_task->regs.ss = 0x10;   // Kernel data segment
        new_task->regs.ds = 0x10;
        new_task->regs.es = 0x10;
        new_task->regs.fs = 0x10;
        new_task->regs.gs = 0x10;
    } else {
        // User mode
        new_task->regs.cs = 0x1B;   // User code segment (0x18 | 3)
        new_task->regs.ss = 0x23;   // User data segment (0x20 | 3)
        new_task->regs.ds = 0x23;
        new_task->regs.es = 0x23;
        new_task->regs.fs = 0x23;
        new_task->regs.gs = 0x23;
    }
    
    // Inizializza gli altri registri a zero
    new_task->regs.eax = 0;
    new_task->regs.ebx = 0;
    new_task->regs.ecx = 0;
    new_task->regs.edx = 0;
    new_task->regs.esi = 0;
    new_task->regs.edi = 0;
    
    // Aggiungi alla ready queue (lista circolare)
    if (!ready_queue) {
        ready_queue = new_task;
        new_task->next = new_task;
    } else {
        // Trova l'ultimo task nella lista
        task_t *tmp = (task_t*)ready_queue;
        while(tmp->next != ready_queue) {
            tmp = tmp->next;
        }
        // Inserisci il nuovo task
        tmp->next = new_task;
        new_task->next = (task_t*)ready_queue;
    }
    
    // Add to all_tasks list
    new_task->parent = all_tasks;
    all_tasks = new_task;

    __asm__ volatile("sti");
    return new_task;
}

// Funzione per terminare il task corrente
void task_exit() {
    __asm__ volatile("cli");
    
    if (!current_task) {
        __asm__ volatile("sti");
        return;
    }
    
    ((task_t*)current_task)->state = TASK_ZOMBIE;
    
    // Rimuovi dalla ready queue
    if (current_task == ready_queue && current_task->next == current_task) {
        // È l'unico task, non possiamo terminarlo
        ((task_t*)current_task)->state = TASK_RUNNING;
        __asm__ volatile("sti");
        return;
    }
    
    task_t *tmp = (task_t*)ready_queue;
    task_t *prev = NULL;
    
    do {
        if (tmp->next == current_task) {
            prev = tmp;
            break;
        }
        tmp = tmp->next;
    } while (tmp != ready_queue);
    
    if (prev) {
        prev->next = ((task_t*)current_task)->next;
        if (current_task == ready_queue) {
            ready_queue = ((task_t*)current_task)->next;
        }
    }
    
    // Forza uno switch al prossimo task
    __asm__ volatile("int $0x20"); // Trigger timer interrupt
    
    __asm__ volatile("sti");
    
    // Non dovremmo mai arrivare qui
    for(;;) __asm__("hlt");
}

// Funzione per ottenere il PID del task corrente
uint32_t get_current_pid() {
    if (current_task) {
        return current_task->id;
    }
    return 0;
}

// Funzione per dormire (yield della CPU)
void task_yield() {
    __asm__ volatile("int $0x20"); // Trigger timer interrupt per forzare context switch
}

// Get task by PID
task_t* get_task_by_pid(uint32_t pid) {
    task_t* curr = all_tasks;
    
    while (curr) {
        if (curr->id == pid) {
            return curr;
        }
        curr = curr->parent;
    }
    
    return NULL;
}

// Set task priority
void set_task_priority(uint32_t pid, uint32_t priority) {
    task_t* task = get_task_by_pid(pid);
    if (task) {
        task->priority = priority;
        // scheduler_set_priority(task, priority); // Uncomment when scheduler integrated
    }
}

// Kill task
void kill_task(uint32_t pid) {
    task_t* task = get_task_by_pid(pid);
    if (task && task != current_task) {
        task->state = TASK_ZOMBIE;
        
        // TODO: Free resources (memory, stack, etc.)
        // TODO: Remove from scheduler
    }
}

// Sleep for milliseconds
void task_sleep(uint32_t ms) {
    // Simple implementation: just yield multiple times
    // TODO: Implement proper timer-based sleep
    uint32_t ticks = ms / 10; // Assuming 100Hz timer
    for (uint32_t i = 0; i < ticks; i++) {
        task_yield();
    }
}