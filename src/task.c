#include "task.h"
#include "mm.h"
#include "heap.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Puntatore al task attualmente in esecuzione
volatile task_t *current_task = NULL;

// Coda dei task pronti (lista concatenata circolare)
volatile task_t *ready_queue = NULL;

// Prossimo ID da assegnare
static uint32_t next_pid = 1;

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

    // Seleziona il prossimo task (round-robin)
    if (current_task->next) {
        current_task = current_task->next;
    } else {
        current_task = (task_t*)ready_queue;
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
}

void init_tasking() {
    __asm__ volatile("cli");

    // Crea il task iniziale per il kernel
    current_task = (task_t*)kmalloc(sizeof(task_t));
    if (!current_task) {
        return; // Out of memory
    }

    current_task->id = next_pid++;
    current_task->state = TASK_RUNNING;
    
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
    current_task->next = current_task;
    ready_queue = current_task;

    __asm__ volatile("sti");
}

task_t* create_task(void (*entry_point)()) {
    __asm__ volatile("cli");

    task_t *new_task = (task_t*)kmalloc(sizeof(task_t));
    if (!new_task) {
        __asm__ volatile("sti");
        return NULL;
    }

    memset(new_task, 0, sizeof(task_t));
    new_task->id = next_pid++;
    new_task->state = TASK_READY;
    
    // Alloca 2 pagine (8KB) per lo stack del nuovo task
    uint32_t stack_phys = pmm_alloc_frame();
    if (!stack_phys) {
        kfree(new_task);
        __asm__ volatile("sti");
        return NULL;
    }
    
    // Mappa lo stack nello spazio di indirizzamento del kernel per ora
    // (in futuro, quando avremo processi separati, useremo address space dedicati)
    uint32_t stack_virt = stack_phys;
    uint32_t stack_top = stack_virt + PAGE_SIZE;
    
    // Alloca una seconda pagina per più spazio stack
    uint32_t stack_phys2 = pmm_alloc_frame();
    if (stack_phys2) {
        stack_top = stack_phys2 + PAGE_SIZE;
    }
    
    // Inizializza i registri per il nuovo task
    new_task->regs.eip = (uint32_t)entry_point;
    new_task->regs.cs = 0x08;   // Kernel code segment
    new_task->regs.eflags = 0x202; // IF=1 (interrupts enabled)
    new_task->regs.esp = stack_top - 16; // Lascia spazio per sicurezza
    new_task->regs.ebp = new_task->regs.esp;
    new_task->regs.ss = 0x10;   // Kernel data segment
    new_task->regs.ds = 0x10;
    new_task->regs.es = 0x10;
    new_task->regs.fs = 0x10;
    new_task->regs.gs = 0x10;
    
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
    
    current_task->state = TASK_ZOMBIE;
    
    // Rimuovi dalla ready queue
    if (current_task == ready_queue && current_task->next == current_task) {
        // È l'unico task, non possiamo terminarlo
        current_task->state = TASK_RUNNING;
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
        prev->next = current_task->next;
        if (current_task == ready_queue) {
            ready_queue = current_task->next;
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