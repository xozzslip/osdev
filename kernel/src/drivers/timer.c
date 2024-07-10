#include "timer.h"
#include "../types.h"
#include "low_level.h"
#include "screen.h"

#define PIT_COMMAND 0x43
#define PIT_DATA_CHANNEL_0 0x40
#define PIT_RESOLUTION_HZ 1193180 // 1.19 MHz

u32 read_pit_count(void)
{
    u32 count = 0;
    // al = channel in bits 6 and 7, remaining bits clear
    outb(0x43, 0b0000000);
    count = inb(0x40); // Low byte
    count |= inb(0x40) << 8; // High byte
    return count;
}

void init_timer(u16 frequency)
{
    u16 divisor = PIT_RESOLUTION_HZ / frequency;
    u8 l = (u8)(divisor & 0xFF);
    u8 h = (u8)((divisor >> 8) & 0xFF);
    outb(PIT_COMMAND, 0x36);
    outb(PIT_DATA_CHANNEL_0, l);
    outb(PIT_DATA_CHANNEL_0, h);
}
