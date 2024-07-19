#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

enum {
    WINDOW_RESIZED,
} typedef KernelEventType;

typedef struct {
    uint8_t* buffer;
    uint32_t width;
    uint32_t height;
} WindowBuffer;

typedef struct {
    uint32_t width;
    uint32_t height;
} WindowResized;

typedef struct {
    KernelEventType type;
    union {
        WindowResized window_resized;
    } data;
} KernelEvent;

typedef struct {
    uint32_t id;
    WindowBuffer window;

} Process;

#endif // PROCESS_H
