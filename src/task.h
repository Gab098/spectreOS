#ifndef TASK_H
#define TASK_H

#include <stdint.h>

// Struttura che definisce lo stato dei registri salvati
typedef struct registers {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Salvati da pusha
    uint32_t eip, cs, eflags, useresp, ss;          // Salvati manualmente
} registers_t;

// Struttura che rappresenta un task
typedef struct task {
    registers_t regs;       // Stato dei registri del task
    uint32_t id;            // ID univoco del task
    struct task *next;      // Puntatore al prossimo task nella lista concatenata
} task_t;

void init_tasking();
void create_task(void (*entry_point)());
void schedule_and_switch(registers_t* regs);

#endif // TASK_H
