#include "usermode.h"
#include "gdt.h"
#include <stdint.h>

// External print function from kernel.c
extern void print(char* str);

// TSS instance (defined in header as extern)
tss_entry_t tss_entry;

// Initialize user mode (set up TSS and user segments in GDT)
void init_usermode() {
    // For now, this is a stub implementation
    // In a full implementation, we would:
    // 1. Set up user code and data segments in the GDT (already done in gdt.h comments)
    // 2. Set up a TSS (Task State Segment) for privilege level switching
    // 3. Load the TSS
    
    // Initialize TSS to zero
    for (uint8_t* p = (uint8_t*)&tss_entry; p < (uint8_t*)(&tss_entry + 1); p++) {
        *p = 0;
    }
    
    // Set up TSS fields
    tss_entry.ss0 = 0x10;  // Kernel data segment
    tss_entry.esp0 = 0;    // Will be set when switching to user mode
    tss_entry.cs = 0x0b;   // Kernel code segment
    tss_entry.ss = 0x13;   // Kernel data segment
    tss_entry.ds = 0x13;
    tss_entry.es = 0x13;
    tss_entry.fs = 0x13;
    tss_entry.gs = 0x13;
    
    // Note: In a full implementation, we would add the TSS to the GDT
    // and load it with ltr instruction. For now, this is just a placeholder.
}

// Switch to user mode and execute function
void switch_to_usermode(void (*user_function)()) {
    (void)user_function;
    
    // This is a stub implementation
    // In a full implementation, we would:
    // 1. Set up a user mode stack
    // 2. Push the user mode segment selectors and stack pointer
    // 3. Push EFLAGS with interrupts enabled
    // 4. Push user code segment and entry point
    // 5. Use iret to switch to ring 3
    
    print("[USERMODE] switch_to_usermode() called (stub)\n");
}

// Helper to enter ring 3
void enter_usermode(uint32_t entry_point, uint32_t stack_ptr) {
    (void)entry_point;
    (void)stack_ptr;
    
    // This is a stub implementation
    // In a full implementation, this would be an assembly routine that:
    // 1. Sets up segment registers for user mode
    // 2. Pushes user stack segment and pointer
    // 3. Pushes EFLAGS
    // 4. Pushes user code segment and entry point
    // 5. Executes iret to jump to ring 3
    
    print("[USERMODE] enter_usermode() called (stub)\n");
}

// Set kernel stack in TSS (used when switching from user to kernel mode)
void set_kernel_stack(uint32_t stack) {
    tss_entry.esp0 = stack;
}
