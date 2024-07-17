#include "drivers/screen.h"
#include "libk/assert.h"
#include "libk/log.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
            klog(FATAL, "address=%u value=%u expected=%u", (uint32_t)b, (*b), value);
            panic();
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
    // assert_memory((void *) 0x590, (void *) 0x6F00, 0x0);
    size_t total_memory = read_memory_size();
    klog(DEBUG, "available memory %uMiB", total_memory / 1024 / 1024);

    if (total_memory < KERNEL_HEAP_LIMIT) {
        klog(FATAL, "not enough memory to allocate kernel heap!");
        panic();
    }
    if (KERNEL_HEAP_LIMIT % CHUNK_SIZE != 0) {
        klog(FATAL, "kernel heap limit must be dividable by CHUNK_SIZE");
        panic();
    }
    size_t total_chunks_count = KERNEL_HEAP_LIMIT / CHUNK_SIZE;
    size_t metadata_size = total_chunks_count * 1; // each chunk consume 1 byte
    if (metadata_size > KERNEL_HEAP_LIMIT) {
        klog(FATAL, "not enough memory to write memory metadata to kernel heap!");
        panic();
    }
    size_t chunks_used_by_metadata = (metadata_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
    klog(DEBUG, "used %d", chunks_used_by_metadata);
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
        klog(FATAL, "failed to allocate enough memory");
        panic();
    }
    if (j - i + 1 > requested_chunks) {
        klog(FATAL, "bug in malloc: callocated more than requested");
        panic();
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
        klog(FATAL, "failed to free pointer %d: pointer is not aligned to CHUNK_SIZE");
        panic();
    }
    if (header->start != p) {
        klog(FATAL, "failed to free pointer %d: allocation header was not found", p);
        panic();
    }
    uint32_t i = (uint32_t)(allocated - KERNEL_HEAP_START) / CHUNK_SIZE;
    uint32_t j = i + header->allocated_chunks;
    klog(DEBUG, "freeing p=%d chunks=%d-%d", allocated, i, j);
    bool* is_free_table = (bool*)KERNEL_HEAP_START;
    for (uint32_t k = i; k <= j; k++) {
        is_free_table[k] = true;
    }
    header->start = 0; // to disallow double free
    header->allocated_chunks = 0;
}

void run_tests()
{
    void* p1 = malloc(3000);
    void* p2 = malloc(1000);
    klog(DEBUG, "p1=%d p2=%d p2-p1=%d", p1, p2, p2 - p1);
    assert((p2 - p1) >= 3000);

    free(p1);
    void* p3 = malloc(100);
    void* p4 = malloc(200);
    void* p5 = malloc(2700);

    klog(DEBUG, "p3=%d", p3);
    assert(p3 == p1);
    assert(p5 > (p2 + 1000));

    free(p2);
    free(p3);
    free(p4);

    void* p6 = malloc(5000);
    void* p7 = malloc(4000);
    assert(p7 == p1);
}
