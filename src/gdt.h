#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// This structure describes a GDT entry.
struct gdt_entry_struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_t;

// This structure describes a GDT pointer.
struct gdt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
typedef struct gdt_ptr_struct gdt_ptr_t;

// GDT entries - now 6 entries to support user mode and TSS
// 0: Null
// 1: Kernel Code (0x08)
// 2: Kernel Data (0x10)
// 3: User Code   (0x18)
// 4: User Data   (0x20)
// 5: TSS         (0x28)
extern gdt_entry_t gdt_entries[6];
extern gdt_ptr_t gdt_ptr;

// Function to initialize the GDT
void init_gdt();

// External assembly function to load the GDT
extern void gdt_flush(uint32_t);

#endif