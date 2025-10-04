#ifndef TASK_H
#define TASK_H

#include <stdint.h>

// Stati del task
#define TASK_RUNNING  0
#define TASK_READY    1
#define TASK_BLOCKED  2
#define TASK_ZOMBIE   3

// Struttura che definisce lo stato dei registri salvati
typedef struct registers {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // General purpose registers
    uint32_t eip, cs, eflags, useresp, ss;           // Interrupt frame
    uint32_t ds, es, fs, gs;                         // Segment registers (opzionale)
} registers_t;

// Struttura che rappresenta un task
typedef struct task {
    registers_t regs;       // Stato dei registri del task
    uint32_t id;            // ID univoco del task (PID)
    uint32_t state;         // Stato del task (RUNNING, READY, etc.)
    struct task *next;      // Puntatore al prossimo task nella ready queue
} task_t;

// Funzioni per la gestione dei task
void init_tasking();
task_t* create_task(void (*entry_point)());
void schedule_and_switch(registers_t* regs);
void task_exit();
void task_yield();
uint32_t get_current_pid();

// Variabile esterna per il task corrente
extern volatile task_t *current_task;
extern volatile task_t *ready_queue;

#endif // TASK_H