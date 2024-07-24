#include "../../include/syscall.h"

void syscall(KR* request)
{
    __asm__ volatile(
        "int $0x80"
        : // no return value
        : "a"((void*)request), // eax
        "b"(1) // ebx
        // Tell the compiler to expect changes in memory and condition codes
        : "memory", "cc");
}
