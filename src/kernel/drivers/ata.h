#ifndef ATA_H
#define ATA_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void init_drive();
void drive_read_blocking(uint32_t lba, uint32_t sectors_count, void* buf);
#endif // ATA_H
