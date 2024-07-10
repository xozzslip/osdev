#include "../types.h"

void set_cursor_offset(unsigned short);
unsigned short get_cursor_offset();
void print_char_at_offset(char, char, unsigned short);
void kprint(char *);
void kprint_u32(u32);
void kprintf(const char *s, ...);
