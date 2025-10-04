#include <stdint.h>
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "mm.h"
#include "task.h"
#include "heap.h"
#include "serial.h"
#include "vfs.h"
#include "io.h"
#include "string.h"

// Current cursor position for kernel printing
int k_current_x = 0;
int k_current_y = 0;

// Function to print a character to the screen
void k_print_char(char c, uint8_t color) {
    volatile unsigned short* vga_buffer = (volatile unsigned short*)0xB8000;
    if (c == '\n') {
        k_current_x = 0;
        k_current_y++;
    } else {
        vga_buffer[k_current_y * 80 + k_current_x] = (unsigned short)c | ((unsigned short)color << 8);
        k_current_x++;
        if (k_current_x >= 80) {
            k_current_x = 0;
            k_current_y++;
        }
    }
    if (k_current_y >= 25) {
        // Scroll up
        for (int i = 0; i < 24 * 80; i++) {
            vga_buffer[i] = vga_buffer[i + 80];
        }
        // Clear last line
        for (int i = 24 * 80; i < 25 * 80; i++) {
            vga_buffer[i] = (unsigned short)' ' | (unsigned short)0x0F00;
        }
        k_current_y = 24;
    }
}

// Simple VGA text mode buffer writer
void print(char* str) {
    int i = 0;
    while (str[i] != 0) {
        k_print_char(str[i], 0x0F);
        i++;
    }
}

// Helper per stampare numeri
void print_hex(uint32_t n) {
    char hex[] = "0123456789ABCDEF";
    for (int i = 28; i >= 0; i -= 4) {
        k_print_char(hex[(n >> i) & 0xF], 0x0F);
    }
}

void print_dec(uint32_t n) {
    if (n == 0) {
        k_print_char('0', 0x0F);
        return;
    }
    
    char buf[16];
    int i = 0;
    while (n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        k_print_char(buf[j], 0x0F);
    }
}

// Task functions for demonstration
void task1_func() {
    uint32_t counter = 0;
    for(;;) {
        k_print_char('1', 0x0A); // Green '1'
        counter++;
        
        // Yield ogni tanto
        if (counter % 5 == 0) {
            task_yield();
        }
        
        // Delay
        for(volatile int i = 0; i < 5000000; i++);
    }
}

void task2_func() {
    uint32_t counter = 0;
    for(;;) {
        k_print_char('2', 0x0B); // Cyan '2'
        counter++;
        
        if (counter % 7 == 0) {
            task_yield();
        }
        
        for(volatile int i = 0; i < 5000000; i++);
    }
}

void task3_func() {
    uint32_t counter = 0;
    for(;;) {
        k_print_char('3', 0x0E); // Yellow '3'
        counter++;
        
        if (counter % 3 == 0) {
            task_yield();
        }
        
        for(volatile int i = 0; i < 5000000; i++);
    }
}

// Test VFS
void test_vfs() {
    print("\n=== Testing VFS ===\n");
    
    // Crea alcuni file di test
    vfs_node_t* test_file = ramfs_create_file(fs_root, "test.txt");
    if (test_file) {
        print("Created file: test.txt\n");
        
        // Scrivi nel file
        char* test_data = "Hello from SpectreOS VFS!";
        uint32_t written = vfs_write(test_file, 0, strlen(test_data), (uint8_t*)test_data);
        print("Wrote ");
        print_dec(written);
        print(" bytes\n");
        
        // Leggi dal file
        char read_buf[128];
        memset(read_buf, 0, 128);
        uint32_t read_bytes = vfs_read(test_file, 0, 127, (uint8_t*)read_buf);
        print("Read ");
        print_dec(read_bytes);
        print(" bytes: ");
        print(read_buf);
        print("\n");
    }
    
    // Crea una directory
    vfs_node_t* test_dir = ramfs_create_dir(fs_root, "testdir");
    if (test_dir) {
        print("Created directory: testdir\n");
        
        // Crea un file dentro la directory
        vfs_node_t* subfile = ramfs_create_file(test_dir, "subfile.txt");
        if (subfile) {
            print("Created file: testdir/subfile.txt\n");
        }
    }
    
    // Lista il contenuto della root
    print("\nContents of /:\n");
    uint32_t i = 0;
    vfs_node_t* node;
    while ((node = vfs_readdir(fs_root, i)) != NULL) {
        print("  ");
        print(node->name);
        if (node->flags & VFS_DIRECTORY) {
            print(" [DIR]");
        }
        print("\n");
        i++;
    }
    
    print("=== VFS Test Complete ===\n\n");
}

// The main entry point for the C part of the kernel
void kmain(unsigned long magic, unsigned long multiboot_info_addr) {
    // Mask all PIC interrupts during setup
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    // Clear screen
    for (int i = 0; i < 80 * 25; i++) {
        k_print_char(' ', 0x0F);
    }
    k_current_x = 0;
    k_current_y = 0;

    print("==================================\n");
    print("  Benvenuto in SpectreOS v0.2\n");
    print("  Multitasking Operating System\n");
    print("==================================\n\n");

    // Initialize serial port for debugging
    print("Inizializzazione Debug Seriale...\n");
    serial_init();
    serial_print("SpectreOS Serial Debug Online!\n");
    print("Debug seriale attivo su COM1.\n");

    // Initialize PMM
    print("Inizializzazione PMM...\n");
    uint32_t multiboot_info_size = *(uint32_t*)multiboot_info_addr;
    pmm_init(multiboot_info_addr, multiboot_info_size);
    print("PMM inizializzato.\n");

    // Initialize VMM
    print("Inizializzazione VMM...\n");
    vmm_init();
    print("VMM inizializzato.\n");

    // Initialize heap
    print("Inizializzazione Heap...\n");
    heap_init(0x400000, 0x100000); // 4MB, size 1MB
    print("Heap inizializzato.\n");

    // Initialize GDT
    print("Inizializzazione GDT...\n");
    init_gdt();
    print("GDT inizializzata.\n");

    // Initialize IDT
    print("Inizializzazione IDT...\n");
    init_idt();
    print("IDT inizializzata.\n");

    // Initialize and remap PIC
    print("Inizializzazione PIC...\n");
    init_pic();
    print("PIC inizializzato.\n");

    // Initialize timer (100 Hz)
    print("Inizializzazione Timer...\n");
    timer_init(100);
    print("Timer inizializzato a 100 Hz.\n");

    // Initialize VFS
    print("Inizializzazione VFS...\n");
    init_ramfs();
    print("VFS (RAMFS) inizializzato.\n");

    // Test VFS
    test_vfs();

    // Initialize multitasking
    print("Inizializzazione Multitasking...\n");
    init_tasking();
    print("Sistema di tasking inizializzato.\n");

    // Enable interrupts!
    __asm__("sti");
    print("Interrupt abilitati.\n\n");

    print("Creazione task di test...\n");
    task_t* t1 = create_task(task1_func);
    task_t* t2 = create_task(task2_func);
    task_t* t3 = create_task(task3_func);
    
    if (t1 && t2 && t3) {
        print("Task creati con successo!\n");
        print("Task 1 PID: "); print_dec(t1->id); print("\n");
        print("Task 2 PID: "); print_dec(t2->id); print("\n");
        print("Task 3 PID: "); print_dec(t3->id); print("\n");
    } else {
        print("ERRORE: Impossibile creare i task!\n");
    }

    print("\n");
    print("==================================\n");
    print(" Kernel avviato con successo!\n");
    print(" Multitasking attivo.\n");
    print(" Premere tasti per testare input.\n");
    print("==================================\n\n");

    // Idle loop - il kernel idle task
    uint32_t idle_counter = 0;
    for (;;) {
        __asm__("hlt"); // Risparmia energia quando non ci sono interrupt
        
        idle_counter++;
        if (idle_counter % 1000000 == 0) {
            // Ogni tanto stampa un punto per mostrare che il kernel è vivo
            serial_print(".");
        }
    }
}