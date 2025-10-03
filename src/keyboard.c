#include "keyboard.h"
#include "io.h"
#include "pic.h"
#include "idt.h" // For struct regs

// External kernel printing function and cursor position
extern void k_print_char(char c, uint8_t color);
extern int k_current_x;
extern int k_current_y;

// Basic scan code to ASCII conversion table (for common keys)
// This is a very simplified version, a real driver would be much more complex.
unsigned char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,   0,   0, ' ', 0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 0
};

// Keyboard interrupt handler
void keyboard_handler(struct regs *r) {
    uint8_t scancode;

    // Read from the keyboard's data port
    scancode = inb(KBD_DATA_PORT);
    io_wait(); // Give the keyboard controller a moment to recover

    // If the top bit of the byte is set, a key has been released
    if (scancode & 0x80) {
        // Key released, ignore for now
    } else {
        // Key pressed
        char c = kbd_us[scancode];
        if (c != 0) {
            if (c == '\b') { // Handle backspace
                if (k_current_x > 0) {
                    k_current_x--;
                    volatile unsigned short* vga_buffer = (volatile unsigned short*)0xB8000;
                    vga_buffer[k_current_y * 80 + k_current_x] = (unsigned short)' ' | ((unsigned short)0x0F << 8); // Erase character
                } else if (k_current_y > 0) {
                    k_current_y--;
                    k_current_x = 79;
                    volatile unsigned short* vga_buffer = (volatile unsigned short*)0xB8000;
                    vga_buffer[k_current_y * 80 + k_current_x] = (unsigned short)' ' | ((unsigned short)0x0F << 8); // Erase character
                }
            } else {
                k_print_char(c, 0x0F);
            }
        }
    }
}

// Function to initialize the keyboard driver (no specific init needed for now)
void init_keyboard() {
    // We just need to ensure the keyboard interrupt is enabled in the PIC
    // This will be done by unmasking IRQ1 in the PIC data register.
    // For now, we'll assume the PIC is already unmasked for IRQ1.
    // (init_pic() handles the remapping, but not specific IRQ unmasking yet)
}
