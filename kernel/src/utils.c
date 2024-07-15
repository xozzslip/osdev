#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define assert2(a) (assert_f((a), "panic"))

#include "drivers/screen.h"
void panic(const char* s, ...)
{
    va_list args;
    va_start(args, s);
    kprintf(s, args);
    va_end(args);
    for (;;) {
        asm volatile("hlt");
    }
}

void assert(const bool assertion, const char* s, ...)
{
    if (!assertion) {
        va_list args;
        va_start(args, s);
        kprintf(s, args);
        va_end(args);
        for (;;) {
            asm volatile("hlt");
        }
    }
}
