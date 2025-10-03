#include "heap.h"
#include <stddef.h>
#include <stdint.h> // Added for uint8_t
#include <string.h> // Added for memset

typedef struct block {
    uint32_t size;
    struct block* next;
    uint32_t magic; // Per debug: 0xDEADBEEF se libero
} block_t;

static block_t* free_list = NULL;
static uint32_t heap_start = 0;
static uint32_t heap_end = 0;

void heap_init(uint32_t start, uint32_t size) {
    heap_start = start;
    heap_end = start + size;
    
    free_list = (block_t*)start;
    free_list->size = size - sizeof(block_t);
    free_list->next = NULL;
    free_list->magic = 0xDEADBEEF;
}

void* kmalloc(uint32_t size) {
    // Allinea a 8 byte
    size = (size + 7) & ~7;
    
    block_t* prev = NULL;
    block_t* curr = free_list;
    
    while(curr) {
        if(curr->size >= size) {
            // Blocco trovato
            if(curr->size > size + sizeof(block_t) + 8) {
                // Dividi il blocco
                block_t* new_block = (block_t*)((uint8_t*)curr + sizeof(block_t) + size);
                new_block->size = curr->size - size - sizeof(block_t);
                new_block->next = curr->next;
                new_block->magic = 0xDEADBEEF;
                curr->size = size;
                curr->next = new_block;
            }
            
            // Rimuovi dalla free list
            if(prev) prev->next = curr->next;
            else free_list = curr->next;
            
            curr->magic = 0xALLOCATED; // Define 0xALLOCATED or use a different value
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
    block->magic = 0xDEADBEEF;
    
    // Aggiungi alla free list (in ordine di indirizzo per coalescing)
    block_t* prev = NULL;
    block_t* curr = free_list;
    
    while(curr && (uint32_t)curr < (uint32_t)block) {
        prev = curr;
        curr = curr->next;
    }
    
    // Inserisci il blocco
    block->next = curr;
    if(prev) prev->next = block;
    else free_list = block;
    
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
    if (block->size >= new_size) {
        // If the current block is large enough, just return it.
        // Could potentially shrink it, but for simplicity, we'll leave it.
        return ptr;
    }

    // Allocate a new block, copy data, and free the old block
    void* new_ptr = kmalloc(new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        kfree(ptr);
    }
    return new_ptr;
}
