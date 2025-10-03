#ifndef IO_H
#define IO_H

#include <stdint.h>

// Read a byte from an I/O port
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

// Write a byte to an I/O port
static inline void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__("outb %0, %1" : : "a"(data), "dN"(port));
}

// Wait for I/O to complete (by writing to an unused port)
static inline void io_wait() {
    // Port 0x80 is used for 'checkpoints' during POST.
    // It is not used by any devices in normal operation.
    outb(0x80, 0);
}

#endif
