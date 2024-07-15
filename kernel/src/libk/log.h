#ifndef LOG_H
#define LOG_H

#include "../drivers/screen.h"

enum {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
} typedef Level;

#define klog(level, format, ...)                        \
    do {                                                \
        const char message[] = #level ": " format "\n"; \
        kprintf(message, ##__VA_ARGS__);                \
    } while (0)



#endif // LOG_H
