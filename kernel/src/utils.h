#if !defined(UTILS_H)
#define UTILS_H
#include <stdbool.h>

#define __ASSERT__(EXP) \
do { if (!(EXP)) { \
    kprintf("PANIC: Assertion \"" #EXP "\" failed in " __FILE__ ":%d\n", __LINE__);  \
    for (;;) { \
        asm volatile("hlt"); \
    }} \
} \
while (0)

void assert(const bool assertion, const char *s, ...);
void panic(const char* s, ...);
#endif
