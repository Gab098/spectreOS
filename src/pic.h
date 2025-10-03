#ifndef PIC_H
#define PIC_H

#include <stdint.h>

// PIC ports
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

// End-of-Interrupt command
#define PIC_EOI 0x20

// Function to initialize and remap the PIC
void init_pic();

// Function to send EOI to PICs
void pic_send_eoi(uint8_t irq);

// Function to initialize the PIT
void timer_init(uint32_t frequency);

#endif
