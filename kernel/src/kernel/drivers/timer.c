#include "timer.h"
#include "low_level.h"
#include "screen.h"
#include <stdint.h>

#define PIT_COMMAND 0x43
#define PIT_DATA_CHANNEL_0 0x40
#define PIT_RESOLUTION_HZ 1193182 // 1.19 MHz

uint32_t read_pit_count(void)
{
    uint32_t count = 0;
    // al = channel in bits 6 and 7, remaining bits clear
    outb(0x43, 0b0000000);
    count = inb(0x40); // Low byte
    count |= inb(0x40) << 8; // High byte
    return count;
}

uint32_t dist(uint32_t a, uint32_t b) {
    if (a >= b) {
        return a - b;
    } else {
        return b - a;
    }
}

uint32_t calculate_best_hertz(uint32_t requested_hz) {
    uint32_t best_hz = PIT_RESOLUTION_HZ;
    uint16_t best_div = 1;
    for (uint16_t div = 1; div < 0xFFFF; div++) {
        uint32_t hz = PIT_RESOLUTION_HZ / div;
        if (dist(hz, requested_hz) < dist(best_hz, requested_hz)) {
            best_hz = hz;
            best_div = div;
        }
    }
    return best_hz; // and best_div?
}

void init_timer()
{
    // uint8_t l = (u8)(best_div & 0xFF);
    // uint8_t h = (u8)((best_div >> 8) & 0xFF);
    outb(PIT_COMMAND, 0x36);
    outb(PIT_DATA_CHANNEL_0, 0xFF);
    outb(PIT_DATA_CHANNEL_0, 0xFF);
}
