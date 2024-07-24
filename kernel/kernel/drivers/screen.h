#ifndef SCREEN_H
#define SCREEN_H
#define VGA_TEXT_WHITE_ON_BLACK 0x0F
#define VGA_TEXT_RED_ON_WHITE 0xF4
#define VGA_TEXT_ADDRESS 0xB8000
void vga_text_write_byte(char ch, char color);
#endif // SCREEN_H
