#ifndef ATA_H
#define ATA_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void enqueue_drive_read(void* buf, size_t size, size_t lba_addr);
void enqueue_drive_write(void* buf, size_t size, size_t lba_addr);
#endif // ATA_H
