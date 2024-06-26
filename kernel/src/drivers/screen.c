#include "low_level.h"
#include "screen.h"
#include "../types.h"

// Miscellaneous Output Register
// http://www.osdever.net/FreeVGA/vga/extreg.htm#3CCR3C2W
#define VGA_REG_MISC_OUTPUT     0x3CC
#define VGA_REG_CRT_ADDRESS     0x3D4
#define VGA_REG_CRT_DATA        0x3D5
// http://www.osdever.net/FreeVGA/vga/crtcreg.htm#0F
#define VGA_IDX_CRT_CURSOR_HIGH 0xE
#define VGA_IDX_CRT_CURSOR_LOW  0xF

// Memory mapped VGA text mode
#define VGA_TEXT_ADDRESS        0xB8000
#define VGA_TEXT_MAX_ROWS       25
#define VGA_TEXT_MAX_COLS       80
#define VGA_TEXT_WHITE_ON_BLACK 0x0F
#define VGA_TEXT_RED_ON_WHITE   0xF4

int calc_col(unsigned short offset) {
    return offset % VGA_TEXT_MAX_COLS;
}

int calc_row(unsigned short offset) {
    int row = offset / VGA_TEXT_MAX_COLS;
    // prevents accessing memory regions not
    // mapped to text mode by passing big offset
    row %= VGA_TEXT_MAX_ROWS;
    return row;
}

unsigned short get_cursor_offset() {
    unsigned char prev_addr = inb(VGA_REG_CRT_ADDRESS);
    outb(VGA_REG_CRT_ADDRESS, VGA_IDX_CRT_CURSOR_HIGH);
    unsigned short current_offset = inb(VGA_REG_CRT_DATA);
    current_offset = current_offset << 8;
    outb(VGA_REG_CRT_ADDRESS, VGA_IDX_CRT_CURSOR_LOW);
    current_offset += inb(VGA_REG_CRT_DATA);
    outb(VGA_REG_CRT_ADDRESS, prev_addr);
    return current_offset;
}

void set_cursor_offset(unsigned short offset) {
    unsigned char prev_addr = inb(VGA_REG_CRT_ADDRESS);
    outb(VGA_REG_CRT_ADDRESS, VGA_IDX_CRT_CURSOR_HIGH);
    outb(VGA_REG_CRT_DATA, (offset & 0xff00) >> 8);
    outb(VGA_REG_CRT_ADDRESS, VGA_IDX_CRT_CURSOR_LOW);
    outb(VGA_REG_CRT_DATA, (offset & 0xff));
    outb(VGA_REG_CRT_ADDRESS, prev_addr);
    return;
}

void print_char_at_offset(char ch, char color, unsigned short offset) {
    // recalculate offset to prevent offset overflow
    // by starting writing on the same screen
    offset = calc_row(offset) * VGA_TEXT_MAX_COLS + calc_col(offset);

    char *video_memory = (char *) (VGA_TEXT_ADDRESS + offset * 2);
    video_memory[0] = ch;
    video_memory[1] = color;
}

void kprint(char *string) {
    int cursor_offset = get_cursor_offset();
    int i = 0;
    while (string[i] != '\0') {
        if (string[i] == '\n') {
            int row = calc_row(cursor_offset);
            row += 1; // new line
            cursor_offset = row * VGA_TEXT_MAX_COLS;
        } else {
            print_char_at_offset(string[i], VGA_TEXT_WHITE_ON_BLACK, cursor_offset);
            cursor_offset++;
        }
        i++;
    }
    set_cursor_offset(cursor_offset);
}

void kprint_u32(u32 value) {
    char s[10] = {0};
    int size = 0;
    for (int i = 0; i < 10; i++) {
        int digit = value % 10;
        s[i] = '0' + digit;
        value /= 10;
        size += 1;
        if (value == 0) {
            break;
        }
    }
    char s2[10] = {0};
    for (int i = 0; i < size; i++) {
        s2[i] = s[size - 1 - i];
    }
    kprint((char *) s2);
}
