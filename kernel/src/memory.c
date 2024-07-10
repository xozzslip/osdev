#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "drivers/screen.h"

#define CHUNKS_LIMIT 10000
#define HEAP_START 0x200000
#define HEAP_LIMIT 10 * 1024 * 1024 // 10MiB

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
  void* start;
} HeapChunk;

HeapChunk* allocated;

void
panic(const char* s, ...)
{
  va_list args;
  va_start(args, s);
  kprintf(s, args);
  va_end(args);
  for (;;) {
    asm volatile("hlt");
  }
}

void
assert_memory(void* from, void* to, uint8_t value)
{
  // assert_memory((void *) 0x590, (void *) 0x6F00, 0x0);
  uint8_t* b = (uint8_t*)from;
  while (b != (uint8_t*)to) {
    if (*b != value) {
      panic(
        "PANIC address=%u value=%u expected=%u\n", (uint32_t)b, (*b), value);
    }
    b++;
  }
}

void
setup_kernel_heap()
{
  allocated = (void*)HEAP_START;
  for (int i = 0; i < CHUNKS_LIMIT; i++) {
    HeapChunk chunk;
    allocated[i] = chunk;
  }
  allocated[0].start = (void*)HEAP_START;
  allocated[0].size = sizeof(HeapChunk) * CHUNKS_LIMIT;

  // according to convention between bootloader and kernel (check memory map)
  AddressRangeDescriptor* ranges = (void*)0x0500;
  AddressRangeDescriptor main_range;

  for (int i = 0; i < 10; i++) {
    AddressRangeDescriptor range = ranges[i];

    if (range.type == 1 && range.length_low >= main_range.length_low) {
      main_range = range;
    }
  }
  uint32_t available_memory = main_range.length_low;
  kprintf("available memory %uMiB\n", available_memory / 1024 / 1024);

  if (available_memory < HEAP_LIMIT) {
    panic("PANIC not enough memory!");
  }
}

void*
malloc(size_t size)
{
  for (int i = 1; i < CHUNKS_LIMIT; i++) {
    HeapChunk prev = allocated[i - 1];
    HeapChunk current = allocated[i];
  }
}
