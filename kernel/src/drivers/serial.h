#if !defined(SERIAL_H)
#define SERIAL_H

#include <stdint.h>
#include <stddef.h>
uint8_t read_serial();
void write_serial(char *s, size_t size);
void write_serial_byte(uint8_t a);
void write_serial_str(const char *s);
void init_serial();
#endif
