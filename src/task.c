#include "task.h"
#include "mm.h" // Per pmm_alloc_frame
#include <stddef.h>
#include <stdint.h>
#include <string.h> // Per memcpy

// Puntatore al task attualmente in esecuzione
volatile task_t *current_task;

// Coda dei task pronti (lista concatenata)
volatile task_t *ready_queue;

// Prossimo ID da assegnare
uint32_t next_pid = 1;

// Funzione per lo scheduling e il context switch
void schedule_and_switch(registers_t* regs) {
    if (current_task) {
        // Salva lo stato del task corrente
        memcpy(&current_task->regs, regs, sizeof(registers_t));
    }

    // Seleziona il prossimo task (round-robin)
    task_t *next_task = current_task->next;
    if (!next_task) {
        next_task = (task_t*)ready_queue;
    }

    current_task = next_task;

    // Carica lo stato del nuovo task nei registri passati
    memcpy(regs, &current_task->regs, sizeof(registers_t));
}

void init_tasking() {
    // Disabilita temporaneamente gli interrupt
    __asm__ volatile("cli");

    // Crea il task iniziale per il kernel
    current_task = (task_t*)pmm_alloc_frame();
    current_task->id = next_pid++;
    // Il task del kernel non ha uno stack separato inizialmente,
    // userà lo stack del kernel. I suoi registri saranno salvati
    // dal primo interrupt del timer.
    memset(&current_task->regs, 0, sizeof(registers_t)); // Inizializza a zero
    current_task->next = NULL;
    ready_queue = current_task;

    // Riabilita gli interrupt
    __asm__ volatile("sti");
}

// Funzione per creare un nuovo task
void create_task(void (*entry_point)()) {
    task_t *new_task = (task_t*)pmm_alloc_frame();
    memset(new_task, 0, sizeof(task_t));
    new_task->id = next_pid++;
    
    // Alloca 2 pagine per lo stack
    uint32_t stack_bottom = pmm_alloc_frame();
    uint32_t stack_top = stack_bottom + PAGE_SIZE;
    
    // Inizializza i registri per il nuovo task
    new_task->regs.eip = (uint32_t)entry_point;
    new_task->regs.cs = 0x08;
    new_task->regs.eflags = 0x202; // IF=1
    new_task->regs.esp = stack_top - 4;
    new_task->regs.ss = 0x10;
    new_task->regs.ds = 0x10;
    new_task->regs.es = 0x10;
    new_task->regs.fs = 0x10;
    new_task->regs.gs = 0x10;
    
    // Aggiungi alla ready queue
    __asm__ volatile("cli");
    if (!ready_queue) {
        ready_queue = new_task;
        new_task->next = new_task; // Circolare
    } else {
        task_t *tmp = (task_t*)ready_queue;
        while(tmp->next != ready_queue) tmp = tmp->next;
        tmp->next = new_task;
        new_task->next = (task_t*)ready_queue;
    }
    __asm__ volatile("sti");
}
