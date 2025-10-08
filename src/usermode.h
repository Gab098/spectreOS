#ifndef USERMODE_H
#define USERMODE_H

#include <stdint.h>
#include "gdt.h"

// GDT segment selectors
#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define USER_CS   0x18
#define USER_DS   0x20

// Initialize user mode (set up TSS and user segments in GDT)
void init_usermode();

// Switch to user mode and execute function
void switch_to_usermode(void (*user_function)());

// Helper to enter ring 3
void enter_usermode(uint32_t entry_point, uint32_t stack_ptr);

// TSS (Task State Segment) structure
typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;      // Kernel stack pointer
    uint32_t ss0;       // Kernel stack segment
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

extern tss_entry_t tss_entry;

// Set kernel stack in TSS (used when switching from user to kernel mode)
void set_kernel_stack(uint32_t stack);

#endif // USERMODE_H