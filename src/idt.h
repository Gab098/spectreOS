#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// A struct to describe an interrupt gate.
struct idt_entry_struct {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

// A struct to describe the IDT pointer.
struct idt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;

// This struct defines the registers pushed by the isr_common_stub.
struct regs {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

// Array of IDT entries.
extern idt_entry_t idt_entries[256];
// Pointer to the IDT.
extern idt_ptr_t   idt_ptr;

// Function to initialize the IDT.
void init_idt();

// External assembly function to load the IDT.
extern void idt_flush(uint32_t);

// External assembly common stub for all ISRs.
extern void isr_common_stub();

// ISRs (Interrupt Service Routines) for the first 32 interrupts (exceptions).
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

// IRQ handlers
extern void irq0_handler();
extern void irq1_handler();

#endif
