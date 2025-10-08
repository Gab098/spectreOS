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
    uint32_t ds, es, fs, gs;                         // Segment registers
} registers_t;

// Struttura che rappresenta un task
typedef struct task {
    registers_t regs;       // Stato dei registri del task
    uint32_t id;            // ID univoco del task (PID)
    uint32_t state;         // Stato del task (RUNNING, READY, etc.)
    uint32_t priority;      // Priorità del task (0-31, 0 = massima)
    uint32_t time_slice;    // Time slice rimanente (in ticks)
    uint32_t cpu_time;      // Tempo CPU totale usato (in ticks)
    uint32_t kernel_stack;  // Stack pointer del kernel per questo task
    uint32_t user_stack;    // Stack pointer user-mode
    uint32_t ring;          // Privilege ring (0 = kernel, 3 = user)
    struct task *next;      // Puntatore al prossimo task nella coda
    struct task *parent;    // Puntatore al task parent (per fork)
    char name[64];          // Nome del task (per debug)
} task_t;

// Funzioni per la gestione dei task
void init_tasking();
task_t* create_task(void (*entry_point)());
task_t* create_task_ex(void (*entry_point)(), uint32_t priority, uint32_t ring, const char* name);
void schedule_and_switch(registers_t* regs);
void task_exit();
void task_yield();
void task_sleep(uint32_t ms);
uint32_t get_current_pid();

// Funzioni avanzate
task_t* get_task_by_pid(uint32_t pid);
void set_task_priority(uint32_t pid, uint32_t priority);
void kill_task(uint32_t pid);

// Variabili esterne per il task corrente
extern volatile task_t *current_task;
extern volatile task_t *ready_queue;

#endif // TASK_H