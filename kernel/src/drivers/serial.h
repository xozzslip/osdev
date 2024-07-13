#if !defined(SERIAL_H)
#define SERIAL_H

#include <stdint.h>
uint8_t read_serial();
void write_serial(uint8_t a);
void init_serial();
#endif
