#if !defined(SERIAL_H)
#define SERIAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
bool init_serial();
void serial_write_byte(char a);

#endif
