#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

void setup_kernel_heap();
void* malloc(size_t requested_size);
void free(void* p);

#endif // MEMORY_H
