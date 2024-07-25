#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#define KERNEL_CODE_START 0x7E00
#define KERNEL_HEAP_START 0x200000 // according to memory map

void setup_kernel_heap();
void* malloc(size_t requested_size);
void free(void* p);

#endif // MEMORY_H
