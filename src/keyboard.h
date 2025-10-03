#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include "idt.h" // For struct regs

// Keyboard controller ports
#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64

// Function to initialize the keyboard driver
void init_keyboard();

// Keyboard interrupt handler
void keyboard_handler(struct regs *r);

#endif
