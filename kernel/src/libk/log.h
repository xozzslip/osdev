#ifndef LOG_H
#define LOG_H

#include "../drivers/screen.h"
#include "../drivers/serial.h"
#include "format.h"

enum {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
} typedef Level;


#define klog(level, format, ...)                  \
    do {                                          \
        char message[] = #level ": " format "\n"; \
        format_and_write_to_vga_text(             \
            message,                              \
            ##__VA_ARGS__);                       \
        format_and_write_to_serial_port(          \
            message,                              \
            ##__VA_ARGS__);                       \
    } while (0)

#endif // LOG_H
