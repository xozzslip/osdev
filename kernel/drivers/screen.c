#include "low_level.h"
#include "screen.h"

#define MAX_ROWS 25
#define MAX_COLS 80


// Miscellaneous Output Register
// http://www.osdever.net/FreeVGA/vga/extreg.htm#3CCR3C2W
#define VGA_REG_MISC_OUTPUT     0x3CC
#define VGA_REG_CRT_ADDRESS     0x3D4
#define VGA_REG_CRT_DATA        0x3D5
// http://www.osdever.net/FreeVGA/vga/crtcreg.htm#0F
#define VGA_IDX_CRT_CURSOR_HIGH 0xE
#define VGA_IDX_CRT_CURSOR_LOW  0xF


unsigned short get_cursor_offset() {
    unsigned char prev_addr = port_byte_in(VGA_REG_CRT_ADDRESS);
    port_byte_out(VGA_REG_CRT_ADDRESS, VGA_IDX_CRT_CURSOR_HIGH);
    unsigned short current_offset = port_byte_in(VGA_REG_CRT_DATA);
    current_offset = current_offset << 8;
    port_byte_out(VGA_REG_CRT_ADDRESS, VGA_IDX_CRT_CURSOR_LOW);
    current_offset += port_byte_in(VGA_REG_CRT_DATA);
    port_byte_out(VGA_REG_CRT_ADDRESS, prev_addr);
    return current_offset;
}

void set_cursor_offset(unsigned short offset) {
    unsigned char prev_addr = port_byte_in(VGA_REG_CRT_ADDRESS);
    port_byte_out(VGA_REG_CRT_ADDRESS, VGA_IDX_CRT_CURSOR_HIGH);
    port_byte_out(VGA_REG_CRT_DATA, (offset & 0xff00) >> 8);
    port_byte_out(VGA_REG_CRT_ADDRESS, VGA_IDX_CRT_CURSOR_LOW);
    port_byte_out(VGA_REG_CRT_DATA, (offset & 0xff));
    port_byte_out(VGA_REG_CRT_ADDRESS, prev_addr);
    return;
}
