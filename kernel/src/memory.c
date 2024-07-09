#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    size_t size;
    void *start;
    bool free;
    char magic[4];
} MemoryBlock;

#define MAX_MEMORY_BLOCKS_COUNT 10000

void setup_kmalloc() {
    MemoryBlock* kernel_memory_address = 0x200000;





}
