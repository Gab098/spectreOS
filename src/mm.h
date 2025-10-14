#ifndef MM_H
#define MM_H

#include "types.h"
#include "multiboot2.h"

#define PAGE_SIZE 4096

// Physical Memory Manager (PMM)
void pmm_init(uint32_t mmap_addr, uint32_t mmap_length);
uint32_t pmm_alloc_frame();
void pmm_free_frame(uint32_t frame);

// Virtual Memory Manager (VMM)

typedef struct {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20;
} page_table_entry_t;

typedef struct {
    page_table_entry_t pages[1024];
} page_table_t;

typedef struct {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20;
} page_directory_entry_t;

typedef struct {
    page_directory_entry_t tables[1024];
} page_directory_t;

// VMM functions
void vmm_init();
void vmm_map_page(uint32_t virt, uint32_t phys);
void vmm_switch_page_directory(page_directory_t *dir);
page_table_entry_t* vmm_get_page(uint32_t address, int make, page_directory_t *dir);

// Page directory management
page_directory_t* clone_page_directory(page_directory_t* src);
void free_page_directory(page_directory_t* dir);

// Global page directories
extern page_directory_t *kernel_directory;
extern page_directory_t *current_directory;

#endif // MM_H