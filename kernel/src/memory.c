#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "drivers/screen.h"

#define CHUNKS_LIMIT 10000
#define HEAP_LIMIT 1024 * 1024 // one MiB
#define HEAP_START 0x200000

// 0	    BaseAddrLow		Low 32 Bits of Base Address
// 4	    BaseAddrHigh	High 32 Bits of Base Address
// 8	    LengthLow		Low 32 Bits of Length in Bytes
// 12	    LengthHigh		High 32 Bits of Length in Bytes
// 16	    Type		    Address type of  this range.
struct AddressRangeDescriptor
{
    uint32_t base_addr_low;
    uint32_t base_addr_high;
    uint32_t length_low;
    uint32_t length_high;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));
typedef struct AddressRangeDescriptor AddressRangeDescriptor;

typedef struct
{
    size_t size;
    void *start;
} HeapChunk;

HeapChunk *allocated;

void assert_memory(void *from, void *to, uint8_t value)
{
    uint8_t *b = (uint8_t *)from;
    while (b != ((uint8_t *)to))
    {
        if ((*b) != value)
        {
            kprintf("PANIC address=%u value=%u expected=%u", (uint32_t)b, (*b), value);
            for (;;)
            {
                asm volatile("hlt");
            }
        }
        b++;
    }
}

void setup_kernel_heap()
{
    allocated = (void *)HEAP_START;
    for (int i = 0; i < CHUNKS_LIMIT; i++)
    {
        HeapChunk chunk;
        allocated[i] = chunk;
    }
    allocated[0].start = (void *)HEAP_START;
    allocated[0].size = sizeof(HeapChunk) * CHUNKS_LIMIT;

    AddressRangeDescriptor *ranges = (void *)0x0500; // according to convention between bootloader and kernel (check memory map)
    for (int i = 0; i < 10; i++)
    {
        AddressRangeDescriptor range = ranges[i];
        kprintf("range %d type=%d al=%u lh=%u\n", i, range.type, range.base_addr_low, range.length_low);
    }
    // assert_memory((void *) 0x590, (void *) 0x6F00, 0);
}

void *malloc(size_t size)
{
    for (int i = 1; i < CHUNKS_LIMIT; i++)
    {
        HeapChunk prev = allocated[i - 1];
        HeapChunk current = allocated[i];
    }
}
