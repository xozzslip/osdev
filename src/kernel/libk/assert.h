#ifndef ASSERT_H
#define ASSERT_H

#include "log.h"

#define __STRINGIFY(x) #x
#define __TOSTRING(x) __STRINGIFY(x)

#define assert(EXP, MSG)                        \
    do {                                        \
        if (!(EXP)) {                           \
            klog(                               \
                FATAL,                          \
                "assertion \"" #EXP             \
                "\" failed: " MSG " (" __FILE__ \
                ":" __TOSTRING(__LINE__) ")");  \
            asm volatile("cli");                \
            asm volatile("hlt");                \
        }                                       \
    } while (0)

#define panic()                         \
    do {                                \
        klog(                           \
            FATAL,                      \
            "kernel panic at " __FILE__ \
            ":" __TOSTRING(__LINE__));  \
        asm volatile("cli");            \
        asm volatile("hlt");            \
    } while (0)

#endif // ASSERT_H
