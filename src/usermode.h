#ifndef USERMODE_H
#define USERMODE_H

#include <stdint.h>
#include "gdt.h"  // For tss_entry_t

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

// Set kernel stack in TSS (used when switching from user to kernel mode)
void set_kernel_stack(uint32_t stack);

#endif // USERMODE_H