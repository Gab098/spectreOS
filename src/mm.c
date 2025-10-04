#include "mm.h"
#include "multiboot2.h"
#include "heap.h"
#include <stddef.h>
#include <string.h>

// External symbol from linker script
extern uint32_t end;

// PMM bitmap
static uint32_t *pmm_bitmap = NULL;
static uint32_t pmm_total_frames = 0;
static uint32_t pmm_bitmap_size = 0;

// Set a bit in the bitmap (mark as used)
static void pmm_set_bit(uint32_t frame) {
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    pmm_bitmap[idx] |= (1 << off);
}

// Clear a bit in the bitmap (mark as free)
static void pmm_clear_bit(uint32_t frame) {
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    pmm_bitmap[idx] &= ~(1 << off);
}

// Test a bit in the bitmap
static uint32_t pmm_test_bit(uint32_t frame) {
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    return (pmm_bitmap[idx] & (1 << off));
}

// Initialize PMM
void pmm_init(uint32_t mmap_addr, uint32_t mmap_length) {
    uint32_t max_mem = 0;
    struct multiboot_tag_mmap *mmap_tag = NULL;

    // Find memory map tag
    for (struct multiboot_tag *tag = (struct multiboot_tag *)mmap_addr;
         (uint8_t *)tag < (uint8_t *)mmap_addr + mmap_length;
         tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7))) {
        if (tag->type == 6) {
            mmap_tag = (struct multiboot_tag_mmap *)tag;
            break;
        }
    }

    if (!mmap_tag) return;

    // Calculate total memory
    for (struct multiboot_mmap_entry *entry = mmap_tag->entries;
         (uint8_t *)entry < (uint8_t *)mmap_tag + mmap_tag->size;
         entry = (struct multiboot_mmap_entry *)((uint8_t *)entry + mmap_tag->entry_size)) {
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t end_addr = entry->addr + entry->len;
            if (end_addr > max_mem) {
                max_mem = end_addr;
            }
        }
    }

    pmm_total_frames = max_mem / PAGE_SIZE;
    pmm_bitmap_size = pmm_total_frames / 8;

    // Place bitmap after kernel
    pmm_bitmap = (uint32_t *)&end;

    // Initially mark all memory as used
    for (uint32_t i = 0; i < pmm_bitmap_size / 4; i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }

    // Free available memory regions
    for (struct multiboot_mmap_entry *entry = mmap_tag->entries;
         (uint8_t *)entry < (uint8_t *)mmap_tag + mmap_tag->size;
         entry = (struct multiboot_mmap_entry *)((uint8_t *)entry + mmap_tag->entry_size)) {
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            for (uint64_t i = 0; i < entry->len; i += PAGE_SIZE) {
                pmm_clear_bit((entry->addr + i) / PAGE_SIZE);
            }
        }
    }

    // Mark kernel and bitmap as used
    uint32_t kernel_start_frame = 0x100000 / PAGE_SIZE;
    uint32_t bitmap_end_addr = (uint32_t)&end + pmm_bitmap_size;
    uint32_t kernel_end_frame = bitmap_end_addr / PAGE_SIZE;
    if (bitmap_end_addr % PAGE_SIZE != 0) {
        kernel_end_frame++;
    }

    for (uint32_t frame = kernel_start_frame; frame < kernel_end_frame; frame++) {
        pmm_set_bit(frame);
    }

    // Always mark first page as used
    pmm_set_bit(0);
}

// Allocate a physical frame
uint32_t pmm_alloc_frame() {
    for (uint32_t frame = 0; frame < pmm_total_frames; frame++) {
        if (!pmm_test_bit(frame)) {
            pmm_set_bit(frame);
            return frame * PAGE_SIZE;
        }
    }
    return 0; // Out of memory
}

// Free a physical frame
void pmm_free_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    if (pmm_test_bit(frame)) {
        pmm_clear_bit(frame);
    }
}

// VMM globals
page_directory_t *kernel_directory = 0;
page_directory_t *current_directory = 0;

// Initialize VMM
void vmm_init() {
    // Allocate page directory
    uint32_t kernel_directory_phys = pmm_alloc_frame();
    kernel_directory = (page_directory_t*)kernel_directory_phys;
    
    // Zero the page directory
    for(int i = 0; i < 1024; i++) {
        kernel_directory->tables[i].present = 0;
        kernel_directory->tables[i].rw = 1;
        kernel_directory->tables[i].user = 0;
        kernel_directory->tables[i].frame = 0;
    }
    
    // Identity map first 8MB (kernel + heap)
    for(int pd_idx = 0; pd_idx < 2; pd_idx++) {
        uint32_t table_phys = pmm_alloc_frame();
        page_table_t *table = (page_table_t*)table_phys;
        
        for(int i = 0; i < 1024; i++) {
            table->pages[i].present = 1;
            table->pages[i].rw = 1;
            table->pages[i].user = 0;
            table->pages[i].frame = (pd_idx * 1024) + i;
        }
        
        kernel_directory->tables[pd_idx].present = 1;
        kernel_directory->tables[pd_idx].rw = 1;
        kernel_directory->tables[pd_idx].user = 0;
        kernel_directory->tables[pd_idx].frame = table_phys >> 12;
    }
    
    vmm_switch_page_directory((page_directory_t*)kernel_directory_phys);
}

// Switch page directory
void vmm_switch_page_directory(page_directory_t *dir) {
    current_directory = dir;
    __asm__ volatile("mov %0, %%cr3":: "r"(dir));
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging
    __asm__ volatile("mov %0, %%cr0":: "r"(cr0));
}

// Get page table entry
page_table_entry_t* vmm_get_page(uint32_t address, int make, page_directory_t *dir) {
    address /= PAGE_SIZE;
    uint32_t table_idx = address / 1024;
    
    if (dir->tables[table_idx].present) {
        page_table_t *table = (page_table_t*)(dir->tables[table_idx].frame * PAGE_SIZE);
        return &table->pages[address % 1024];
    } else if(make) {
        uint32_t tmp = pmm_alloc_frame();
        dir->tables[table_idx].present = 1;
        dir->tables[table_idx].rw = 1;
        dir->tables[table_idx].user = 0;
        dir->tables[table_idx].frame = tmp >> 12;
        
        page_table_t *table = (page_table_t*)tmp;
        for(int i=0; i<1024; i++) {
            table->pages[i].present = 0;
        }
        return &table->pages[address % 1024];
    }
    return 0;
}

// Map virtual address to physical address
void vmm_map_page(uint32_t virt, uint32_t phys) {
    page_table_entry_t *page = vmm_get_page(virt, 1, kernel_directory);
    if (page) {
        page->present = 1;
        page->rw = 1;
        page->user = 0;
        page->frame = phys >> 12;
    }
}

// Clone a page table
static page_table_t* clone_page_table(page_table_t *src, uint32_t *phys_addr) {
    // Allocate new page table
    uint32_t table_phys = pmm_alloc_frame();
    page_table_t *table = (page_table_t*)table_phys;
    
    if (phys_addr) {
        *phys_addr = table_phys;
    }
    
    // Copy all entries
    for (int i = 0; i < 1024; i++) {
        if (!src->pages[i].present) {
            table->pages[i].present = 0;
            continue;
        }
        
        // Allocate new frame for copy-on-write
        uint32_t new_frame = pmm_alloc_frame();
        
        // Copy page data
        memcpy((void*)new_frame, 
               (void*)(src->pages[i].frame * PAGE_SIZE), 
               PAGE_SIZE);
        
        // Set up new page entry
        table->pages[i].present = src->pages[i].present;
        table->pages[i].rw = src->pages[i].rw;
        table->pages[i].user = src->pages[i].user;
        table->pages[i].accessed = 0;
        table->pages[i].dirty = 0;
        table->pages[i].frame = new_frame >> 12;
    }
    
    return table;
}

// Clone a page directory
page_directory_t* clone_page_directory(page_directory_t *src) {
    // Allocate new page directory
    uint32_t dir_phys = pmm_alloc_frame();
    page_directory_t *dir = (page_directory_t*)dir_phys;
    
    // Zero it out
    memset(dir, 0, sizeof(page_directory_t));
    
    // Copy each page table
    for (int i = 0; i < 1024; i++) {
        if (!src->tables[i].present) {
            continue;
        }
        
        // Get source table
        page_table_t *src_table = (page_table_t*)(src->tables[i].frame * PAGE_SIZE);
        
        // Clone it
        uint32_t new_table_phys;
        clone_page_table(src_table, &new_table_phys);
        
        // Set up directory entry
        dir->tables[i].present = 1;
        dir->tables[i].rw = src->tables[i].rw;
        dir->tables[i].user = src->tables[i].user;
        dir->tables[i].frame = new_table_phys >> 12;
    }
    
// Funzione helper per copiare una page table
static page_table_t* clone_page_table(page_table_t* src, uint32_t* phys_addr) {
    // Alloca una nuova page table
    uint32_t phys = pmm_alloc_frame();
    if (!phys) return NULL;
    
    page_table_t* table = (page_table_t*)phys;
    *phys_addr = phys;
    
    // Copia tutte le entry
    for (int i = 0; i < 1024; i++) {
        if (src->pages[i].present) {
            // Alloca un nuovo frame per questa pagina
            uint32_t new_frame = pmm_alloc_frame();
            if (!new_frame) {
                // Gestione errore: dovremmo liberare le pagine già allocate
                return NULL;
            }
            
            // Copia i dati dalla pagina sorgente
            memcpy((void*)new_frame, 
                   (void*)(src->pages[i].frame * PAGE_SIZE), 
                   PAGE_SIZE);
            
            // Configura la nuova entry
            table->pages[i].present = src->pages[i].present;
            table->pages[i].rw = src->pages[i].rw;
            table->pages[i].user = src->pages[i].user;
            table->pages[i].accessed = 0;
            table->pages[i].dirty = 0;
            table->pages[i].frame = new_frame >> 12;
        } else {
            table->pages[i].present = 0;
        }
    }
    
    return table;
}

// Clona una page directory (per creare un nuovo processo)
page_directory_t* clone_page_directory(page_directory_t* src) {
    // Alloca la nuova page directory
    uint32_t phys = pmm_alloc_frame();
    if (!phys) return NULL;
    
    page_directory_t* dir = (page_directory_t*)phys;
    
    // Inizializza tutte le entry a 0
    for (int i = 0; i < 1024; i++) {
        dir->tables[i].present = 0;
    }
    
    // Copia le page table
    for (int i = 0; i < 1024; i++) {
        if (src->tables[i].present) {
            page_table_t* src_table = (page_table_t*)(src->tables[i].frame * PAGE_SIZE);
            uint32_t table_phys;
            
            page_table_t* new_table = clone_page_table(src_table, &table_phys);
            if (!new_table) {
                // Gestione errore: dovremmo liberare tutto
                return NULL;
            }
            
            dir->tables[i].present = 1;
            dir->tables[i].rw = src->tables[i].rw;
            dir->tables[i].user = src->tables[i].user;
            dir->tables[i].frame = table_phys >> 12;
        }
    }
    
    return dir;
}

// Funzione per liberare una page directory
void free_page_directory(page_directory_t* dir) {
    if (!dir) return;
    
    // Libera tutte le page table e i loro frame
    for (int i = 0; i < 1024; i++) {
        if (dir->tables[i].present) {
            page_table_t* table = (page_table_t*)(dir->tables[i].frame * PAGE_SIZE);
            
            // Libera tutti i frame nella page table
            for (int j = 0; j < 1024; j++) {
                if (table->pages[j].present) {
                    pmm_free_frame(table->pages[j].frame * PAGE_SIZE);
                }
            }
            
            // Libera la page table stessa
            pmm_free_frame(dir->tables[i].frame * PAGE_SIZE);
        }
    }
    
    // Libera la page directory
    pmm_free_frame((uint32_t)dir);
}

    return dir;
}