#include "heap.h"
#include "string.h"
#include <stddef.h>
#include <stdint.h>

#define HEAP_MAGIC_FREE      0xDEADBEEF
#define HEAP_MAGIC_ALLOCATED 0xABADCAFE

typedef struct block {
    uint32_t size;
    struct block* next;
    uint32_t magic; // Per debug: HEAP_MAGIC_FREE se libero, HEAP_MAGIC_ALLOCATED se allocato
} block_t;

static block_t* free_list = NULL;
static uint32_t heap_start = 0;
static uint32_t heap_end = 0;
static uint32_t heap_max = 0;

void heap_init(uint32_t start, uint32_t size) {
    heap_start = start;
    heap_end = start + size;
    heap_max = heap_end;
    
    free_list = (block_t*)start;
    free_list->size = size - sizeof(block_t);
    free_list->next = NULL;
    free_list->magic = HEAP_MAGIC_FREE;
}

void* kmalloc(uint32_t size) {
    if (size == 0) return NULL;
    
    // Allinea a 8 byte
    size = (size + 7) & ~7;
    
    block_t* prev = NULL;
    block_t* curr = free_list;
    
    while(curr) {
        // Verifica integrità
        if (curr->magic != HEAP_MAGIC_FREE) {
            // Corruzione heap!
            return NULL;
        }
        
        if(curr->size >= size) {
            // Blocco trovato
            if(curr->size > size + sizeof(block_t) + 8) {
                // Dividi il blocco se abbastanza grande
                block_t* new_block = (block_t*)((uint8_t*)curr + sizeof(block_t) + size);
                new_block->size = curr->size - size - sizeof(block_t);
                new_block->next = curr->next;
                new_block->magic = HEAP_MAGIC_FREE;
                
                curr->size = size;
                curr->next = new_block;
            }
            
            // Rimuovi dalla free list
            if(prev) {
                prev->next = curr->next;
            } else {
                free_list = curr->next;
            }
            
            curr->magic = HEAP_MAGIC_ALLOCATED;
            curr->next = NULL; // Importante: azzera next quando allocato
            
            return (void*)((uint8_t*)curr + sizeof(block_t));
        }
        prev = curr;
        curr = curr->next;
    }
    
    return NULL; // Out of memory
}

void kfree(void* ptr) {
    if(!ptr) return;
    
    block_t* block = (block_t*)((uint8_t*)ptr - sizeof(block_t));
    
    // Verifica che il blocco sia stato allocato
    if (block->magic != HEAP_MAGIC_ALLOCATED) {
        // Double free o corruzione!
        return;
    }
    
    block->magic = HEAP_MAGIC_FREE;
    
    // Aggiungi alla free list (in ordine di indirizzo per coalescing)
    block_t* prev = NULL;
    block_t* curr = free_list;
    
    while(curr && (uint32_t)curr < (uint32_t)block) {
        prev = curr;
        curr = curr->next;
    }
    
    // Inserisci il blocco
    block->next = curr;
    if(prev) {
        prev->next = block;
    } else {
        free_list = block;
    }
    
    // Coalescing con il blocco successivo
    if(curr && (uint8_t*)block + sizeof(block_t) + block->size == (uint8_t*)curr) {
        block->size += sizeof(block_t) + curr->size;
        block->next = curr->next;
    }
    
    // Coalescing con il blocco precedente
    if(prev && (uint8_t*)prev + sizeof(block_t) + prev->size == (uint8_t*)block) {
        prev->size += sizeof(block_t) + block->size;
        prev->next = block->next;
    }
}

void* krealloc(void* ptr, uint32_t new_size) {
    if (!ptr) {
        return kmalloc(new_size);
    }
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    block_t* block = (block_t*)((uint8_t*)ptr - sizeof(block_t));
    
    // Verifica integrità
    if (block->magic != HEAP_MAGIC_ALLOCATED) {
        return NULL;
    }
    
    if (block->size >= new_size) {
        // Il blocco corrente è abbastanza grande
        return ptr;
    }

    // Alloca un nuovo blocco, copia i dati e libera il vecchio
    void* new_ptr = kmalloc(new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        kfree(ptr);
    }
    return new_ptr;
}

// Funzione di debug per verificare lo stato dell'heap
void heap_dump() {
    extern void print(char*);
    extern void print_dec(uint32_t);
    extern void print_hex(uint32_t);
    
    print("\n=== Heap Dump ===\n");
    print("Heap Start: 0x"); print_hex(heap_start); print("\n");
    print("Heap End: 0x"); print_hex(heap_end); print("\n");
    print("Free blocks:\n");
    
    block_t* curr = free_list;
    int count = 0;
    
    while(curr && count < 20) { // Limita a 20 per sicurezza
        print("  Block at 0x"); print_hex((uint32_t)curr);
        print(" Size: "); print_dec(curr->size);
        print(" Magic: 0x"); print_hex(curr->magic);
        print("\n");
        
        curr = curr->next;
        count++;
    }
    
    print("=================\n");
}