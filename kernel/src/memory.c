#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "drivers/screen.h"
#include "utils.h"

void run_tests();

// Kernel memory allocation implemented
// using fixed chunks allocation

#define CHUNK_SIZE 512 // 512 bytes
#define KERNEL_HEAP_START 0x200000 // according to memory map
#define KERNEL_HEAP_LIMIT 10 * 1024 * 1024 // 10 MiB

// this struct is written by BIOS
struct AddressRangeDescriptor {
    uint32_t base_addr_low; // Low 32 Bits of Base Address
    uint32_t base_addr_high; // High 32 Bits of Base Address
    uint32_t length_low; // Low 32 Bits of Length in Bytes
    uint32_t length_high; // High 32 Bits of Length in Bytes
    uint32_t type; // Address type of  this range.
    uint32_t reserved;
} __attribute__((packed));
typedef struct AddressRangeDescriptor AddressRangeDescriptor;

void assert_memory(void* from, void* to, uint8_t value)
{
    // usage example: check that memory is zeroed
    // assert_memory((void *) 0x590, (void *) 0x6F00, 0x0);
    uint8_t* b = (uint8_t*)from;
    while (b != (uint8_t*)to) {
        if (*b != value) {
            panic("PANIC address=%u value=%u expected=%u\n", (uint32_t)b, (*b),
                value);
        }
        b++;
    }
}

// returns memory size in bytes
// this information is fetched during real mode and written to specific location
// according to convention between bootloader and kernel (check memory map)
size_t read_memory_size()
{
    AddressRangeDescriptor* ranges = (void*)0x0500;
    AddressRangeDescriptor main_range;

    for (int i = 0; i < 100; i++) {
        AddressRangeDescriptor range = ranges[i];

        if (range.type == 1 && range.length_low >= main_range.length_low) {
            main_range = range;
        }
        if (range.type == 0) {
            break;
        }
    }
    uint32_t total_memory = main_range.length_low;
    return total_memory;
}

void setup_kernel_heap()
{
    assert_memory((void *) 0x590, (void *) 0x6F00, 0x0);
    size_t total_memory = read_memory_size();
    kprintf("available memory %uMiB\n", total_memory / 1024 / 1024);

    if (total_memory < KERNEL_HEAP_LIMIT) {
        panic("PANIC not enough memory to allocate kernel heap!");
    }
    if (KERNEL_HEAP_LIMIT % CHUNK_SIZE != 0) {
        panic("PANIC kernel heap limit must be dividable by CHUNK_SIZE");
    }
    size_t total_chunks_count = KERNEL_HEAP_LIMIT / CHUNK_SIZE;
    size_t metadata_size = total_chunks_count * 1; // each chunk consume 1 byte
    if (metadata_size > KERNEL_HEAP_LIMIT) {
        panic("PANIC not enough memory to write memory metadata to kernel heap!");
    }
    size_t chunks_used_by_metadata = (metadata_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
    kprintf("used %d\n", chunks_used_by_metadata);
    bool* is_free_table = (bool*)KERNEL_HEAP_START;
    for (uint32_t chunk_no = 0; chunk_no < total_chunks_count; chunk_no++) {
        if (chunk_no < chunks_used_by_metadata) {
            is_free_table[chunk_no] = false;
        } else {
            is_free_table[chunk_no] = true;
        }
    }
    run_tests();
}

typedef struct {
    void* start;
    size_t allocated_chunks;
} AllocationHeader;

void* malloc(size_t requested_size)
{
    size_t size = requested_size + sizeof(AllocationHeader);
    size_t requested_chunks = (size + CHUNK_SIZE - 1) / CHUNK_SIZE;
    size_t total_chunks_count = KERNEL_HEAP_LIMIT / CHUNK_SIZE;
    bool* is_free_table = (bool*)KERNEL_HEAP_START;
    uint32_t i = 0;
    uint32_t j = 0;
    bool found = false;
    while (i < total_chunks_count) {
        if (!is_free_table[i]) {
            i++;
            continue;
        }
        j = i;
        while (j < total_chunks_count) {
            if (!is_free_table[j]) {
                break;
            }
            if (j - i + 1 == requested_chunks) {
                found = true;
                break;
            }
            j++;
        }
        if (found) {
            break;
        }
        i = j + 1;
    }
    if (!found) {
        panic("PANIC failed to allocate enough memory");
    }
    if (j - i + 1 > requested_chunks) {
        panic("PANIC bug in malloc: callocated more than requested\n");
    }
    for (uint32_t k = 0; k < requested_chunks; k++) {
        is_free_table[i + k] = false;
    }
    void* allocated = (void*)KERNEL_HEAP_START + i * CHUNK_SIZE;
    void* p = allocated + sizeof(AllocationHeader);
    *(AllocationHeader*)allocated = (AllocationHeader) {
        .start = p,
        .allocated_chunks = requested_chunks,
    };
    return p;
}

void free(void* p)
{
    if (p == NULL) {
        return;
    }
    void* allocated = p - sizeof(AllocationHeader);

    AllocationHeader* header = (AllocationHeader*)allocated;
    if ((uint32_t)allocated % CHUNK_SIZE != 0) {
        panic("PANIC failed to free pointer %d: pointer is not aligned to CHUNK_SIZE");
    }
    if (header->start != p) {
        panic("PANIC failed to free pointer %d: allocation header was not found", p);
    }
    uint32_t i = (uint32_t)(allocated - KERNEL_HEAP_START) / CHUNK_SIZE;
    uint32_t j = i + header->allocated_chunks;
    kprintf("freeing p=%d chunks=%d-%d\n", allocated, i, j);
    bool* is_free_table = (bool*)KERNEL_HEAP_START;
    for (uint32_t k = i; k <= j; k++) {
        is_free_table[k] = true;
    }
    header->start = 0; // to disallow double free
    header->allocated_chunks = 0;
}

void run_tests()
{
    void* p1 = malloc(1000);
    void* p2 = malloc(1000);
    kprintf("p1=%d p2=%d\n", p1, p2);
    free(p1);
    void* p3 = malloc(100);
    kprintf("p3=%d\n", p3);
}
