#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

void serial_init();
int serial_received();
char serial_read();
int serial_transmit_empty();
void serial_write(char c);
void serial_print(const char* str);

// Macro per debug
#define DEBUG(msg) serial_print("[DEBUG] " msg "\n")
// DEBUG_HEX requires sprintf, which is not available yet. Will add later if sprintf is implemented.
// #define DEBUG_HEX(val) { char buf[11]; sprintf(buf, "0x%08X", val); serial_print(buf); }

#endif
