#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void* memcpy(void* dest, const void* src, size_t n)
{
    char* dest_char = (char*)dest;
    const char* src_char = (const char*)src;
    for (size_t i = 0; i < n; i++) {
        dest_char[i] = src_char[i];
    }
    return dest;
}

#endif // UTILS_H
