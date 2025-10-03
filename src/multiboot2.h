#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H

#include <stdint.h>

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_mmap_entry {
        uint64_t addr;
        uint64_t len;
        uint32_t type;
        uint32_t zero;
    } entries[];
};

#define MULTIBOOT_MEMORY_AVAILABLE 1

#endif // MULTIBOOT2_H
