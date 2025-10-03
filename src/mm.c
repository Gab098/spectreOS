#include "mm.h"
#include "multiboot2.h"
#include <stddef.h>

// Dichiarazione esterna del simbolo 'end' dal linker script
extern uint32_t end;

// Bitmap per il PMM
static uint32_t *pmm_bitmap = NULL;
static uint32_t pmm_total_frames = 0;
static uint32_t pmm_bitmap_size = 0;

// Funzione per impostare un bit nella bitmap
static void pmm_set_bit(uint32_t frame) {
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    pmm_bitmap[idx] |= (1 << off);
}

// Funzione per liberare un bit nella bitmap
static void pmm_clear_bit(uint32_t frame) {
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    pmm_bitmap[idx] &= ~(1 << off);
}

// Funzione per testare un bit nella bitmap
static uint32_t pmm_test_bit(uint32_t frame) {
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    return (pmm_bitmap[idx] & (1 << off));
}

// Inizializza il PMM
void pmm_init(uint32_t mmap_addr, uint32_t mmap_length) {
    uint32_t max_mem = 0;
    struct multiboot_tag_mmap *mmap_tag = NULL;

    // Trova il tag della mappa di memoria
    for (struct multiboot_tag *tag = (struct multiboot_tag *)mmap_addr;
         (uint8_t *)tag < (uint8_t *)mmap_addr + mmap_length;
         tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7))) {
        if (tag->type == 6) { // Tipo 6 è il tag mmap
            mmap_tag = (struct multiboot_tag_mmap *)tag;
            break;
        }
    }

    if (!mmap_tag) return; // Mappa di memoria non trovata

    // Calcola la memoria totale per determinare la dimensione della bitmap
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
    pmm_bitmap_size = pmm_total_frames / 8; // 1 bit per frame

    // Posiziona la bitmap dopo la fine del kernel
    pmm_bitmap = (uint32_t *)&end;

    // Inizialmente, segna tutta la memoria come non disponibile (tutti i bit a 1)
    for (uint32_t i = 0; i < pmm_bitmap_size / 4; i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }

    // Libera i frame disponibili secondo la mappa di memoria (imposta i bit a 0)
    for (struct multiboot_mmap_entry *entry = mmap_tag->entries;
         (uint8_t *)entry < (uint8_t *)mmap_tag + mmap_tag->size;
         entry = (struct multiboot_mmap_entry *)((uint8_t *)entry + mmap_tag->entry_size)) {
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            for (uint64_t i = 0; i < entry->len; i += PAGE_SIZE) {
                pmm_clear_bit((entry->addr + i) / PAGE_SIZE);
            }
        }
    }

    // Ora, marca esplicitamente come occupate le pagine usate dal kernel.
    // Assumiamo che il kernel sia caricato a 1MB (0x100000).
    uint32_t kernel_start_frame = 0x100000 / PAGE_SIZE;
    uint32_t bitmap_end_addr = (uint32_t)&end + pmm_bitmap_size;
    uint32_t kernel_end_frame = bitmap_end_addr / PAGE_SIZE;
    if (bitmap_end_addr % PAGE_SIZE != 0) {
        kernel_end_frame++;
    }

    for (uint32_t frame = kernel_start_frame; frame < kernel_end_frame; frame++) {
        pmm_set_bit(frame);
    }

    // Per sicurezza, marca sempre come occupata la prima pagina (frame 0)
    // che contiene strutture critiche del BIOS/real mode.
    pmm_set_bit(0);
}

// Alloca un frame di memoria fisica
uint32_t pmm_alloc_frame() {
    for (uint32_t frame = 0; frame < pmm_total_frames; frame++) {
        if (!pmm_test_bit(frame)) {
            pmm_set_bit(frame);
            return frame * PAGE_SIZE;
        }
    }
    return 0; // Nessun frame libero
}

// Libera un frame di memoria fisica
void pmm_free_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    if (pmm_test_bit(frame)) {
        pmm_clear_bit(frame);
    }
}

// VMM implementation
page_directory_t *kernel_directory = 0;
page_directory_t *current_directory = 0;

void vmm_init() {
    // Alloca la page directory PRIMA di abilitare paging
    uint32_t kernel_directory_phys = pmm_alloc_frame();
    kernel_directory = (page_directory_t*)kernel_directory_phys;
    
    // Azzera la page directory
    for(int i = 0; i < 1024; i++) {
        kernel_directory->tables[i].present = 0;
        kernel_directory->tables[i].rw = 1;
        kernel_directory->tables[i].user = 0;
        kernel_directory->tables[i].frame = 0;
    }
    
    // Identity map i primi 8MB invece di 4MB (per includere kernel + heap)
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

void vmm_switch_page_directory(page_directory_t *dir) {
    current_directory = dir;
    __asm__ volatile("mov %0, %%cr3":: "r"(dir));
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging
    __asm__ volatile("mov %0, %%cr0":: "r"(cr0));
}

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
        for(int i=0; i<1024; i++) table->pages[i].present = 0;
        return &table->pages[address % 1024];
    } else {
        return 0;
    }
}

void vmm_map_page(uint32_t virt, uint32_t phys) {
    page_table_entry_t *page = vmm_get_page(virt, 1, kernel_directory);
    page->present = 1;
    page->rw = 1;
    page->user = 0;
    page->frame = phys >> 12;
}
