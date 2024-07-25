#ifndef ATA_H
#define ATA_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void init_drive();
void drive_read_blocking(uint32_t lba, uint32_t sectors_count, void* buf);
void drive_send_read_command(uint32_t lba, uint32_t sectors_count, void* buf);
bool is_drive_bus_ready();
void drive_receive_sector();
#endif // ATA_H
