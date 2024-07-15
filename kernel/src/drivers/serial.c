#include "low_level.h"
#include "../libk/assert.h"
#include "../libk/log.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define PORT 0x3f8 // COM1

void init_serial()
{
    outb(PORT + 1, 0x00); // Disable all interrupts
    outb(PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + 1, 0x00); //                  (hi byte)
    outb(PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    outb(PORT + 4, 0x1E); // Set in loopback mode, test the serial chip
    outb(PORT + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(PORT + 0) != 0xAE) {
        klog(FATAL, "faulty serial port: loopback check failed\n");
        panic();
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(PORT + 4, 0x0F);
}

bool serial_received()
{
    return inb(PORT + 5) & 1;
}

uint8_t read_serial()
{
    while (serial_received() == 0) {
        // spinwait
    }
    return inb(PORT);
}

bool is_transmit_empty()
{
    return inb(PORT + 5) & 0x20;
}

void write_serial_byte(uint8_t a)
{
    while (is_transmit_empty() == 0) {
        // spinwait
    }
    outb(PORT, a);
}


void write_serial(char *s, size_t size) {
    for (int i = 0; i < size; i++) {
        write_serial_byte(s[i]);
    }
}

void write_serial_str(const char *s) {
    int i = 0;
    while (true) {
        write_serial_byte(s[i]);
        if (s[i] == '\0') {
            break;
        }
        i++;
    }
}
