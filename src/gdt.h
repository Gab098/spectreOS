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

// Function to initialize the GDT
void init_gdt();

// External assembly function to load the GDT
extern void gdt_flush(uint32_t);

#endif
