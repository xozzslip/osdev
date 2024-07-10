#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "drivers/screen.h"


#define CHUNKS_LIMIT 10000
#define HEAP_LIMIT 1024 * 1024  // one MiB
#define HEAP_START 0x200000

typedef struct
{
    size_t size;
    void *start;
} HeapChunk;

HeapChunk allocated[1000];




void setup_kernel_heap() {

}
