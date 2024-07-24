#ifndef FORMAT_H
#define FORMAT_H
#include <stdbool.h>
void format_and_write(bool (*write_byte)(char), char* format, ...);
void format_and_write_to_vga_text(char *format, ...);
void format_and_write_to_serial_port(char *format, ...);
#endif // FORMAT_H
