#include "pic.h"
#include "io.h" // We'll need this for inb/outb functions

// Initialization Command Words (ICWs) for PIC remapping
#define ICW1_ICW4       0x01 // ICW4 (not) present
#define ICW1_SINGLE     0x02 // Single (cascade) mode
#define ICW1_INTERVAL4  0x04 // Call address interval 4 (8)
#define ICW1_LEVEL      0x08 // Level triggered (edge) mode
#define ICW1_INIT       0x10 // Initialization - required!

#define ICW4_8086       0x01 // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO       0x02 // Auto (normal) EOI
#define ICW4_BUF_SLAVE  0x08 // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C // Buffered mode/master
#define ICW4_SFNM       0x10 // Special fully nested (not)

// Function to initialize and remap the PIC
void init_pic() {
    uint8_t a1, a2;

    // Save masks
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    // Start initialization sequence (in cascade mode)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // Remap PIC1 to 0x20 (IRQ 32)
    outb(PIC1_DATA, 0x20);
    io_wait();
    // Remap PIC2 to 0x28 (IRQ 40)
    outb(PIC2_DATA, 0x28);
    io_wait();

    // Tell master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC1_DATA, 0x04);
    io_wait();
    // Tell slave PIC its cascade identity (0000 0010)
    outb(PIC2_DATA, 0x02);
    io_wait();

    // Set 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Restore masks (or set new ones)
    // Unmask IRQ0 (timer) and IRQ1 (keyboard)
    outb(PIC1_DATA, 0xFC); 
    // Mask all IRQs on PIC2
    outb(PIC2_DATA, 0xFF);
}

// Function to send EOI to PICs
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

void timer_init(uint32_t frequency) {
    // The PIT operates at a base frequency of 1193180 Hz.
    uint32_t divisor = 1193180 / frequency;

    // Send the command byte.
    outb(0x43, 0x36);

    // Send the frequency divisor.
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);
    outb(0x40, l);
    outb(0x40, h);
}
