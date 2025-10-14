#include "gdt.h"
#include "string.h"    // For memset

// GDT entries array - 6 entries for user mode support
gdt_entry_t gdt_entries[6];  // Changed from 3 to 6
// GDT pointer
gdt_ptr_t   gdt_ptr;

// Function to set a GDT entry
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

// Write TSS
static void write_tss(int32_t num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t)&tss_entry;
    uint32_t limit = sizeof(tss_entry);
    
    // Add TSS descriptor to GDT
    gdt_set_gate(num, base, limit, 0xE9, 0x00);
    
    // Zero out the TSS
    memset(&tss_entry, 0, sizeof(tss_entry));
    
    tss_entry.ss0 = ss0;    // Kernel stack segment
    tss_entry.esp0 = esp0;   // Kernel stack pointer
    
    // Set IO permissions
    tss_entry.iomap_base = sizeof(tss_entry);
}

// Initialization function for the GDT
void init_gdt() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 6) - 1;  // Changed from 3 to 6
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    // Null segment (0x00)
    gdt_set_gate(0, 0, 0, 0, 0);
    
    // Kernel code segment (0x08): base=0, limit=4GB, 32-bit, granularity=4KB
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    
    // Kernel data segment (0x10): base=0, limit=4GB, 32-bit, granularity=4KB
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    
    // User code segment (0x18): base=0, limit=4GB, 32-bit, ring 3
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    
    // User data segment (0x20): base=0, limit=4GB, 32-bit, ring 3  
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    
    // TSS segment (0x28) - will be set up later
    write_tss(5, 0x10, 0x0);  // ss0 = kernel data segment, esp0 will be set later

    // Load the GDT
    gdt_flush((uint32_t)&gdt_ptr);
    
    // Load TSS
    __asm__ volatile("ltr %%ax" : : "a"(0x2B));  // 0x28 | 0x03 = 0x2B
}