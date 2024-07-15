#if !defined(UTILS_H)
#define UTILS_H
#include <stdbool.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

enum {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
} typedef Level;

#define klog(level, format, ...) \
do { \
    const char message[] = #level ": " format "\n"; \
    kprintf(message, ##__VA_ARGS__); \
} while(0)

#define assert(EXP) \
do { if (!(EXP)) { \
    klog(FATAL, "assertion failed at " __FILE__ ":" TOSTRING(__LINE__) " \"" #EXP "\""); \
    for (;;) { \
        asm volatile("hlt"); \
    }} \
} while (0)






void panic(const char* s, ...);
#endif
