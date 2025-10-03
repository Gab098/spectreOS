#include "idt.h"
#include "gdt.h" // For GDT_CODE_SEGMENT_SELECTOR
#include "pic.h" // For pic_send_eoi
#include "keyboard.h" // For keyboard_handler
#include "task.h"     // For schedule_and_switch
#include <stdint.h>

// External function from kernel.c for printing
extern void k_print_char(char c, uint8_t color);
extern void print(char* str);

// Defined in gdt.c, but we need the selector value
#define GDT_CODE_SEGMENT_SELECTOR 0x08

idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

// Dichiarazioni esterne per gli handler ISR in assembly
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
extern void isr32(); // Timer
extern void isr33(); // Keyboard

// Function to set an IDT entry
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entries[num].base_low    = base & 0xFFFF;
    idt_entries[num].base_high   = (base >> 16) & 0xFFFF;
    idt_entries[num].selector    = selector;
    idt_entries[num].zero        = 0;
    idt_entries[num].flags       = flags;
}

// Generic interrupt handler for exceptions (ISR 0-31) and IRQs
void isr_handler(struct regs *r) {
    // Handle timer interrupt specifically
    if (r->int_no == 0x20) { // Timer IRQ (remapped to 0x20)
        schedule_and_switch(r);
        pic_send_eoi(r->int_no); // Send EOI to master PIC for IRQ0
        return;
    }
    // Handle keyboard interrupt specifically
    if (r->int_no == 0x21) { // Keyboard IRQ (remapped to 0x21)
        keyboard_handler(r);
        pic_send_eoi(r->int_no); // Send EOI for keyboard
        return;
    }

    // For all other interrupts (exceptions and other IRQs)
    k_print_char('I', 0x0F);
    k_print_char('N', 0x0F);
    k_print_char('T', 0x0F);
    k_print_char(':', 0x0F);
    k_print_char((r->int_no / 100) % 10 + '0', 0x0F);
    k_print_char((r->int_no / 10) % 10 + '0', 0x0F);
    k_print_char((r->int_no % 10) + '0', 0x0F);
    k_print_char(' ', 0x0F);

    // If this is an IRQ, send EOI
    if (r->int_no >= 32 && r->int_no <= 47) {
        pic_send_eoi(r->int_no);
    }

    // If it's an exception, print a message.
    if (r->int_no < 32) {
        if (r->int_no == 14) { // Page Fault
            uint32_t faulting_address;
            __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address));
            
            print("\nPAGE FAULT at address: ");
            char hex_digits[] = "0123456789ABCDEF";
            for (int i = 7; i >= 0; i--) {
                k_print_char(hex_digits[(faulting_address >> (i * 4)) & 0xF], 0x0C);
            }
            print("\n");
            for(;;) __asm__("hlt");
        } else {
            print("\nEXCEPTION!\n");
        }
    }
}

// Initialization function for the IDT
void init_idt() {
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // Clear out the IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Set up the first 32 entries for exceptions
    idt_set_gate(0, (uint32_t)isr0, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, GDT_CODE_SEGMENT_SELECTOR, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, GDT_CODE_SEGMENT_SELECTOR, 0x8E);

    // IRQ handlers
    idt_set_gate(32, (uint32_t)isr32, GDT_CODE_SEGMENT_SELECTOR, 0x8E); // Timer
    idt_set_gate(33, (uint32_t)isr33, GDT_CODE_SEGMENT_SELECTOR, 0x8E); // Keyboard

    // Load the IDT
    idt_flush((uint32_t)&idt_ptr);
}
