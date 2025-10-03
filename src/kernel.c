#include <stdint.h> // For uint8_t

// Current cursor position for kernel printing (made global for keyboard.c)
int k_current_x = 0;
int k_current_y = 0;

// Function to print a character to the screen and advance cursor (for kernel messages)
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
    if (k_current_y >= 25) { // Scroll up if at bottom
        // Scroll all lines up by one
        for (int i = 0; i < 24 * 80; i++) {
            vga_buffer[i] = vga_buffer[i + 80];
        }
        // Clear the last line
        for (int i = 24 * 80; i < 25 * 80; i++) {
            vga_buffer[i] = (unsigned short)' ' | (unsigned short)0x0F00;
        }
        k_current_y = 24; // Keep cursor on the last line
    }
}

// A simple VGA text mode buffer writer
void print(char* str) {
    int i = 0;
    while (str[i] != 0) {
        k_print_char(str[i], 0x0F); // White on Black
        i++;
    }
}

#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "mm.h"
#include "task.h"
#include "heap.h"   // Added for heap_init
#include "serial.h" // Added for serial_init

#include "io.h" // For outb

// The main entry point for the C part of the kernel
void kmain(unsigned long magic, unsigned long multiboot_info_addr) {
    // Mask all PIC interrupts to prevent spurious IRQs during setup
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    // Clear screen using k_print_char
    for (int i = 0; i < 80 * 25; i++) {
        k_print_char(' ', 0x0F);
    }
    k_current_x = 0; // Reset cursor after clearing
    k_current_y = 0;

    print("Benvenuto in SpectreOS!\n");

    // PMM must be initialized BEFORE VMM
    print("Inizializzazione PMM...\n");
    uint32_t multiboot_info_size = *(uint32_t*)multiboot_info_addr;
    pmm_init(multiboot_info_addr, multiboot_info_size);
    print("PMM inizializzato.\n");

    print("Inizializzazione VMM...\n");
    vmm_init();
    print("VMM inizializzato.\n");

    // Inizializza heap (dopo VMM)
    print("Inizializzazione Heap...\n");
    heap_init(0x400000, 0x100000); // Heap a 4MB, dimensione 1MB
    print("Heap inizializzato.\n");

    // Inizializza porta seriale per debug
    print("Inizializzazione Debug Seriale...\n");
    serial_init();
    serial_print("SpectreOS Serial Debug Online!\n");
    print("Debug seriale attivo su COM1.\n");

    print("Inizializzazione GDT...\n");
    init_gdt();
    print("GDT inizializzata.\n");

    print("Inizializzazione IDT...\n");
    init_idt();
    print("IDT inizializzata.\n");

    print("Inizializzazione PIC...\n");
    init_pic(); // Initialize and remap the PIC
    print("PIC inizializzato.\n");

    print("Inizializzazione Timer...\n");
    timer_init(100); // 100 Hz
    print("Timer inizializzato.\n");

    __asm__("sti"); // Enable interrupts globally!
    print("Interrupt abilitati.\n");

    // Reset cursor to a new line for user input after kernel messages
    k_current_x = 0;
    k_current_y++; // Move to the next line for user input

    print("Kernel in esecuzione. Inizializzazione multitasking...\n");
    init_tasking();

    // Creiamo due task di esempio
    void task1_func();
    void task2_func();
    create_task(task1_func);
    create_task(task2_func);

    print("Multitasking avviato.\n");

    // Il kernel ora può entrare in un loop idle, lo scheduler farà il resto
    for (;;) {
        __asm__("hlt");
    }
}

void task1_func() {
    for(;;) {
        k_print_char('A', 0x0A);
        for(volatile int i=0; i<10000000; i++); // Delay
    }
}

void task2_func() {
    for(;;) {
        k_print_char('B', 0x0B);
        for(volatile int i=0; i<10000000; i++); // Delay
    }
}
