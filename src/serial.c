#include "io.h"
#include "serial.h" // Include for DEBUG macros
#include <stdarg.h> // For va_list, va_start, va_end
#include <string.h> // For strlen
#include <stdint.h> // For uint32_t

#define SERIAL_PORT 0x3F8 // COM1

void serial_init() {
    outb(SERIAL_PORT + 1, 0x00);    // Disabilita interrupt
    outb(SERIAL_PORT + 3, 0x80);    // Abilita DLAB
    outb(SERIAL_PORT + 0, 0x03);    // Divisore baud rate (38400)
    outb(SERIAL_PORT + 1, 0x00);    
    outb(SERIAL_PORT + 3, 0x03);    // 8 bit, no parity, 1 stop
    outb(SERIAL_PORT + 2, 0xC7);    // Abilita FIFO
    outb(SERIAL_PORT + 4, 0x0B);    // IRQs abilitati, RTS/DSR set
}

int serial_received() {
    return inb(SERIAL_PORT + 5) & 1;
}

char serial_read() {
    while (!serial_received());
    return inb(SERIAL_PORT);
}

int serial_transmit_empty() {
    return inb(SERIAL_PORT + 5) & 0x20;
}

void serial_write(char c) {
    while (!serial_transmit_empty());
    outb(SERIAL_PORT, c);
}

void serial_print(const char* str) {
    while(*str) {
        serial_write(*str++);
    }
}

// Basic sprintf implementation for DEBUG_HEX
static void itoa(uint32_t value, char* buffer, int base) {
    char* p = buffer;
    char* p1, *p2;
    uint32_t divisor = 1;

    if (base == 10) {
        if (value < 0) {
            *p++ = '-';
            value = -value;
        }
        while (value / divisor >= 10)
            divisor *= 10;
    } else if (base == 16) {
        // For hex, we'll just handle it directly
        // This is a very simplified version, not a full sprintf
        for (int i = 7; i >= 0; i--) {
            uint8_t nibble = (value >> (i * 4)) & 0xF;
            *p++ = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        }
        *p = '\0';
        return;
    }

    do {
        *p++ = "0123456789abcdef"[value % base];
        value /= base;
    } while (value > 0);

    *p = '\0';
    p1 = buffer;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

// A very basic sprintf for hex values only, for DEBUG_HEX
void sprintf_hex(char* buf, const char* format, uint32_t val) {
    // This is a highly simplified sprintf for the specific DEBUG_HEX format "0x%08X"
    // It does not handle general sprintf formats.
    if (strcmp(format, "0x%08X") == 0) {
        strcpy(buf, "0x");
        itoa(val, buf + 2, 16); // Convert to hex and append
    } else {
        // Fallback for unsupported formats, or implement more robust sprintf
        strcpy(buf, "UNSUPPORTED_SPRINTF_FORMAT");
    }
}

// Re-define DEBUG_HEX with the basic sprintf_hex
#undef DEBUG_HEX
#define DEBUG_HEX(val) { \
    char buf[11]; \
    sprintf_hex(buf, "0x%08X", val); \
    serial_print(buf); \
}
