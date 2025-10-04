#include "process.h"
#include "task.h"
#include "mm.h"
#include "heap.h"
#include "string.h"

static process_t* process_list = NULL;
static process_t* current_process = NULL;
static uint32_t next_pid = 1;

process_t* create_process(const char* name, void (*entry)()) {
    __asm__ volatile("cli");
    
    process_t* proc = (process_t*)kmalloc(sizeof(process_t));
    if (!proc) {
        __asm__ volatile("sti");
        return NULL;
    }
    
    // Inizializza il processo
    proc->pid = next_pid++;
    proc->state = PROCESS_READY;
    
    // Copia il nome
    strncpy(proc->name, name, 63);
    proc->name[63] = '\0';
    
    // Crea un nuovo address space (clone della kernel directory)
    proc->page_dir = clone_page_directory(kernel_directory);
    if (!proc->page_dir) {
        kfree(proc);
        __asm__ volatile("sti");
        return NULL;
    }
    
    // Crea il thread principale del processo
    proc->main_thread = create_task(entry);
    if (!proc->main_thread) {
        free_page_directory(proc->page_dir);
        kfree(proc);
        __asm__ volatile("sti");
        return NULL;
    }
    
    // Collega il thread al processo (richiede aggiunta di campo process in task_t)
    // proc->main_thread->process = proc;
    
    // Aggiungi alla lista dei processi
    proc->next = process_list;
    process_list = proc;
    
    __asm__ volatile("sti");
    return proc;
}

process_t* get_process_by_pid(uint32_t pid) {
    process_t* curr = process_list;
    
    while (curr) {
        if (curr->pid == pid) {
            return curr;
        }
        curr = curr->next;
    }
    
    return NULL;
}

void kill_process(uint32_t pid) {
    __asm__ volatile("cli");
    
    process_t* proc = get_process_by_pid(pid);
    if (!proc) {
        __asm__ volatile("sti");
        return;
    }
    
    // Marca il processo come zombie
    proc->state = PROCESS_ZOMBIE;
    
    // TODO: Terminare tutti i thread del processo
    // TODO: Liberare le risorse (memory, file descriptors, etc.)
    
    // Rimuovi dalla lista dei processi
    if (process_list == proc) {
        process_list = proc->next;
    } else {
        process_t* curr = process_list;
        while (curr && curr->next != proc) {
            curr = curr->next;
        }
        if (curr) {
            curr->next = proc->next;
        }
    }
    
    // Libera la page directory
    if (proc->page_dir != kernel_directory) {
        free_page_directory(proc->page_dir);
    }
    
    // Libera la struttura del processo
    kfree(proc);
    
    __asm__ volatile("sti");
}

process_t* get_current_process() {
    return current_process;
}

void set_current_process(process_t* proc) {
    current_process = proc;
    
    // Switch alla page directory del processo
    if (proc && proc->page_dir) {
        vmm_switch_page_directory(proc->page_dir);
    }
}

// Lista tutti i processi (debug)
void list_processes() {
    extern void print(char* str);
    extern void k_print_char(char c, uint8_t color);
    
    print("\n=== Process List ===\n");
    
    process_t* curr = process_list;
    while (curr) {
        print("PID: ");
        
        // Stampa il PID (conversione semplice)
        uint32_t pid = curr->pid;
        if (pid == 0) {
            k_print_char('0', 0x0F);
        } else {
            char buf[16];
            int i = 0;
            while (pid > 0) {
                buf[i++] = '0' + (pid % 10);
                pid /= 10;
            }
            // Stampa al contrario
            for (int j = i - 1; j >= 0; j--) {
                k_print_char(buf[j], 0x0F);
            }
        }
        
        print(" Name: ");
        print(curr->name);
        print(" State: ");
        
        switch(curr->state) {
            case PROCESS_RUNNING: print("RUNNING"); break;
            case PROCESS_READY:   print("READY");   break;
            case PROCESS_BLOCKED: print("BLOCKED"); break;
            case PROCESS_ZOMBIE:  print("ZOMBIE");  break;
            default: print("UNKNOWN");
        }
        
        print("\n");
        curr = curr->next;
    }
    
    print("===================\n");
}