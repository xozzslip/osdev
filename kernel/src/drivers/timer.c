#include "../types.h"
#include "low_level.h"
#include "screen.h"
#include "timer.h"

#define PIT_COMMAND        0x43
#define PIT_DATA_CHANNEL_0 0x40
#define PIT_RESOLUTION_HZ  1193180 // 1.19 MHz

u32 read_pit_count(void) {
	u32 count = 0;
	// al = channel in bits 6 and 7, remaining bits clear
	port_byte_out(0x43,0b0000000);
	count = port_byte_in(0x40);		    // Low byte
	count |= port_byte_in(0x40)<<8;		// High byte
	return count;
}

void init_timer(u16 frequency) {
    u16 divisor = PIT_RESOLUTION_HZ / frequency;
    u8 l = (u8)(divisor & 0xFF);
    u8 h = (u8)((divisor>>8) & 0xFF);
    port_byte_out(PIT_COMMAND, 0x36);
    port_byte_out(PIT_DATA_CHANNEL_0, l);
    port_byte_out(PIT_DATA_CHANNEL_0, h);
}
