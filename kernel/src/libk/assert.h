#ifndef ASSERT_H
#define ASSERT_H

#include "log.h"
#define __STRINGIFY(x) #x
#define __TOSTRING(x) __STRINGIFY(x)

#define assert(EXP)                                        \
    do {                                                   \
        if (!(EXP)) {                                      \
            klog(                                          \
                FATAL,                                     \
                "assertion failed at " __FILE__            \
                ":" __TOSTRING(__LINE__) " \"" #EXP "\""); \
            asm volatile("cli");                           \
            asm volatile("hlt");                           \
        }                                                  \
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
